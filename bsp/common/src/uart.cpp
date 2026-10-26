#include "mcu.hpp"
#include "uart.hpp"

#include BSP_LL_DMA_HEADER
#include BSP_LL_USART_HEADER

namespace bsp {

void Uart::write_byte(std::uint8_t byte) noexcept {
    while (LL_USART_IsActiveFlag_TXE(instance_) == 0U) {}

    LL_USART_TransmitData8(instance_, byte);
}

void Uart::write(std::string_view text) noexcept {
    for (const char character : text) {
        write_byte(static_cast<std::uint8_t>(character));
    }

    while (LL_USART_IsActiveFlag_TC(instance_) == 0U) {}
}

void Uart::write_line(std::string_view text) noexcept {
    write(text);
    write("\r\n");
}

UartStatus Uart::start_write_dma(const std::uint8_t* data, std::size_t size) noexcept {
    if ((data == nullptr) || (size == 0U)) return UartStatus::ok;

    if (transmit_active_) return UartStatus::busy;

    transmit_active_ = true;
    transmit_complete_ = false;
    transmit_error_ = false;

    // disable channel before DMA configuration
    transmit_dma_.disable();
    // RAM source address
    LL_DMA_SetMemoryAddress(transmit_dma_.controller,
                            transmit_dma_.channel,
                            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(data)));
    // USART2 transmit data register destination.
    LL_DMA_SetPeriphAddress(transmit_dma_.controller,
                            transmit_dma_.channel,
#if defined(BSP_MCU_FAMILY_STM32F4)
                            LL_USART_DMA_GetRegAddr(instance_));
#else
                            LL_USART_DMA_GetRegAddr(instance_, LL_USART_DMA_REG_DATA_TRANSMIT));
#endif
    // Number of bytes to transfer.
    LL_DMA_SetDataLength(transmit_dma_.controller, transmit_dma_.channel, size);
    // Clear flags left over from an earlier transfer.
    transmit_dma_.clear_flags();
    // Enable DMA completion and error interrupts.
    transmit_dma_.enable_interrupts();
    // Allow USART2 to request DMA transfers.
    LL_USART_EnableDMAReq_TX(instance_);
    // Start the transfer.
    transmit_dma_.enable();

    return UartStatus::ok;
}

UartStatus Uart::write_dma(const std::uint8_t* data, std::size_t size) noexcept {
    const UartStatus status{start_write_dma(data, size)};

    if (status != UartStatus::ok) return status;

    while (!transmit_complete_.load() && !transmit_error_.load()) {
        __WFI();
    }

    if (transmit_error_.load()) return UartStatus::dma_error;

    /*
     * DMA completion means the final byte was written to the USART.
     * Wait until USART finishes physically transmitting it.
     */
    while (LL_USART_IsActiveFlag_TC(instance_) == 0U);

    return UartStatus::ok;
}

UartStatus Uart::write_dma(std::string_view text) noexcept {
    return write_dma(reinterpret_cast<const std::uint8_t*>(text.data()),
                     text.size());
}

UartStatus Uart::write_line_dma(std::string_view text) noexcept {
    const UartStatus status{write_dma(text)};
    if (status != UartStatus::ok) return status;
    static constexpr std::uint8_t line_ending[]{'\r', '\n'};
    return write_dma(line_ending, sizeof(line_ending));
}

bool Uart::transmitting_dma() const noexcept {
    return transmit_active_.load();
}

void Uart::handle_transmit_dma_complete() noexcept {
    transmit_dma_.disable();
    LL_USART_DisableDMAReq_TX(instance_);
    transmit_complete_.store(true);
    transmit_active_.store(false);
}

void Uart::handle_transmit_dma_error() noexcept {
    transmit_dma_.disable();
    LL_USART_DisableDMAReq_TX(instance_);
    transmit_error_.store(true);
    transmit_active_.store(false);
}

} // namespace bsp
