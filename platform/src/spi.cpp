#include "spi.hpp"

#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_spi.h"

#include <cstdint>

namespace platform {

SpiStatus Spi::transfer(const std::uint8_t* transmit, std::uint8_t* receive, std::size_t size) noexcept {
    if (size == 0U) return SpiStatus::ok;
    for (std::size_t index{0U}; index < size; ++index) {
        // Wait until the SPI transmit data register can accept another byte.
        if (!wait_for_transmit_empty()) return SpiStatus::timeout;
        /*
         * If the caller did not provide a transmit buffer, send a dummy byte
         * to generate the SPI clock.
         */
        const std::uint8_t transmit_byte{transmit != nullptr ? transmit[index] : dummy_byte};
        LL_SPI_TransmitData8(instance_, transmit_byte);
        // Wait until one byte has been received.
        if (!wait_for_receive_ready()) return SpiStatus::timeout;
        // Read the receive data register.
        const std::uint8_t receive_byte{LL_SPI_ReceiveData8(instance_)};
        // Store the received byte only if the caller supplied a receive buffer.
        if (receive != nullptr) receive[index] = receive_byte;
    }
    if (!wait_for_not_busy()) return SpiStatus::timeout;
    return SpiStatus::ok;
}

SpiStatus Spi::transfer_dma(const std::uint8_t* transmit, std::uint8_t* receive, std::size_t size) noexcept {
    if (size == 0U) return SpiStatus::ok;
    // Prevent another DMA transfer from modifying the DMA channels while this transfer is active.
    bool expected{false};
    if (!dma_active_.compare_exchange_strong(expected, true)) return SpiStatus::busy;
    transmit_complete_.store(false);
    receive_complete_.store(false);
    dma_error_.store(false);
    // DMA channel configuration registers must only be changed while the channels are disabled.
    LL_DMA_DisableChannel(transmit_dma_.controller, transmit_dma_.channel);
    LL_DMA_DisableChannel(receive_dma_.controller, receive_dma_.channel);
    /*
     * Configure the SPI data register as the peripheral address for both DMA channels.
     * TX DMA writes to the data register.
     * RX DMA reads from the same data register.
     */
    const std::uint32_t spi_data_register{LL_SPI_DMA_GetRegAddr(instance_)};
    LL_DMA_SetPeriphAddress(transmit_dma_.controller, transmit_dma_.channel, spi_data_register);
    LL_DMA_SetPeriphAddress(receive_dma_.controller, receive_dma_.channel, spi_data_register);
    /*
     * If the caller does not provide a transmit buffer, repeatedly transmit
     * a dummy byte to generate the SPI clock.
     */
    const std::uint8_t* transmit_source{transmit != nullptr ? transmit : &dummy_transmit_};
    LL_DMA_SetMemoryAddress(transmit_dma_.controller,
                            transmit_dma_.channel,
                            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(transmit_source)));
    LL_DMA_SetMemoryIncMode(transmit_dma_.controller,
                            transmit_dma_.channel,
                            transmit != nullptr ? LL_DMA_MEMORY_INCREMENT : LL_DMA_MEMORY_NOINCREMENT);
    /*
     * If the caller does not provide a receive buffer, repeatedly overwrite
     * a dummy byte so that the SPI receive register is still drained.
     */
    std::uint8_t* receive_destination{receive != nullptr ? receive : &dummy_receive_};
    LL_DMA_SetMemoryAddress(receive_dma_.controller,
                            receive_dma_.channel,
                            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(receive_destination)));
    LL_DMA_SetMemoryIncMode(receive_dma_.controller,
                            receive_dma_.channel,
                            receive != nullptr ? LL_DMA_MEMORY_INCREMENT : LL_DMA_MEMORY_NOINCREMENT);
    // TX and RX perform to the same number of transfers
    LL_DMA_SetDataLength(transmit_dma_.controller, transmit_dma_.channel, static_cast<std::uint32_t>(size));
    LL_DMA_SetDataLength(receive_dma_.controller, receive_dma_.channel, static_cast<std::uint32_t>(size));
    // Enable transfer-complete and transfer-error interrupts.
    LL_DMA_EnableIT_TC(transmit_dma_.controller, transmit_dma_.channel);
    LL_DMA_EnableIT_TE(transmit_dma_.controller, transmit_dma_.channel);
    LL_DMA_EnableIT_TC(receive_dma_.controller, receive_dma_.channel);
    LL_DMA_EnableIT_TE(receive_dma_.controller, receive_dma_.channel);
    // Start both DMA channels before allowing SPI to issue requests.
    LL_DMA_EnableChannel(receive_dma_.controller, receive_dma_.channel);
    LL_DMA_EnableChannel(transmit_dma_.controller, transmit_dma_.channel);
    // Allow SPI to generate hardware DMA requests.
    LL_SPI_EnableDMAReq_RX(instance_);
    LL_SPI_EnableDMAReq_TX(instance_);
    // Wait for both DMA channels to complete.
    /*
     * TODO: When FreeRTOS is added, this is the section that should become a
     * blocking task notification instead of a polling loop.
     */
    while ((!transmit_complete_.load() || !receive_complete_.load()) && !dma_error_.load());
    // Prevent the SPI peripheral from generating additional DMA requests.
    LL_SPI_DisableDMAReq_TX(instance_);
    LL_SPI_DisableDMAReq_RX(instance_);
    LL_DMA_DisableChannel(transmit_dma_.controller, transmit_dma_.channel);
    LL_DMA_DisableChannel(receive_dma_.controller, receive_dma_.channel);

    if (dma_error_.load()) {
        dma_active_.store(false);
        return SpiStatus::dma_error;
    }
    /*
     * DMA completion means the final byte has moved through the SPI data
     * register. The SPI shift register may still be transmitting the final
     * bits, so wait until the peripheral is no longer busy.
     */
    if (!wait_for_not_busy()) {
        dma_active_.store(false);
        return SpiStatus::timeout;
    }
    dma_active_.store(false);
    return SpiStatus::ok;
}

bool Spi::wait_for_transmit_empty() const noexcept {
    std::uint32_t timeout{timeout_iterations};
    while (LL_SPI_IsActiveFlag_TXE(instance_) == 0U) {
        if (timeout == 0U) return false;
        --timeout;
    }
    return true;
}

bool Spi::wait_for_receive_ready() const noexcept {
    std::uint32_t timeout{timeout_iterations};
    while (LL_SPI_IsActiveFlag_RXNE(instance_) == 0U) {
        if (timeout == 0U) return false;
        --timeout;
    }
    return true;
}

bool Spi::wait_for_not_busy() const noexcept {
    std::uint32_t timeout{timeout_iterations};
    while (LL_SPI_IsActiveFlag_BSY(instance_) != 0U) {
        if (timeout == 0U) return false;
        --timeout;
    }
    return true;
}

void Spi::handle_transmit_dma_complete() noexcept {
    transmit_complete_.store(true);
}

void Spi::handle_receive_dma_complete() noexcept {
    receive_complete_.store(true);
}

void Spi::handle_dma_error() noexcept {
    dma_error_.store(true);
}

} // namespace platform