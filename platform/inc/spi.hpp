#ifndef PLATFORM_INC_SPI_HPP
#define PLATFORM_INC_SPI_HPP

#include "dma.hpp"
#include "stm32l4xx.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace platform {

/**
 * @brief Result of an SPI transfer.
 */
enum class SpiStatus : std::uint8_t {
    ok,
    timeout,
    busy,
    dma_error
};

/**
 * @brief Polling full-duplex SPI master.
 */
class Spi final {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Constructs an SPI interface.
     *
     * @param instance SPI peripheral instance.
     * @param transmit_dma DMA channel used for SPI transmission.
     * @param receive_dma DMA channel used for SPI reception.
     */
    constexpr Spi(SPI_TypeDef* instance, DmaChannel transmit_dma, DmaChannel receive_dma) noexcept
        : instance_{instance}, transmit_dma_{transmit_dma}, receive_dma_{receive_dma} {}

    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Performs a polling SPI transfer.
     *
     * @param transmit Transmit buffer, or nullptr to send dummy bytes.
     * @param receive Receive buffer, or nullptr to discard received bytes.
     * @param size Number of bytes to transfer.
     * @return Status of the transfer.
     */
    [[nodiscard]] SpiStatus transfer(const std::uint8_t* transmit,
                                     std::uint8_t* receive,
                                     std::size_t size) noexcept;

    /**
     * @brief Performs an SPI transfer using DMA.
     *
     * This function starts the TX and RX DMA channels and blocks until the
     * DMA transfer completes.
     *
     * @param transmit Transmit buffer, or nullptr to send dummy bytes.
     * @param receive Receive buffer, or nullptr to discard received bytes.
     * @param size Number of bytes to transfer.
     * @return Status of the transfer.
     */
    [[nodiscard]] SpiStatus transfer_dma(const std::uint8_t* transmit,
                                         std::uint8_t* receive,
                                         std::size_t size) noexcept;

    /**
     * @brief Handles completion of the SPI transmit DMA channel.
     */
    void handle_transmit_dma_complete() noexcept;

    /**
     * @brief Handles completion of the SPI receive DMA channel.
     */
    void handle_receive_dma_complete() noexcept;

    /**
     * @brief Handles an SPI DMA transfer error.
     */
    void handle_dma_error() noexcept;

  private:
    [[nodiscard]] bool wait_for_transmit_empty() const noexcept;
    [[nodiscard]] bool wait_for_receive_ready() const noexcept;
    [[nodiscard]] bool wait_for_not_busy() const noexcept;

    SPI_TypeDef* instance_;

    DmaChannel transmit_dma_;
    DmaChannel receive_dma_;

    std::atomic_bool dma_active_{false};
    std::atomic_bool transmit_complete_{false};
    std::atomic_bool receive_complete_{false};
    std::atomic_bool dma_error_{false};

    std::uint8_t dummy_transmit_{0x00U};
    std::uint8_t dummy_receive_{0x00U};

    static constexpr std::uint8_t dummy_byte{0x00U};
    static constexpr std::uint32_t timeout_iterations{100000U};
};

} // namespace platform

#endif // PLATFORM_INC_SPI_HPP