#ifndef PLATFORM_INC_UART_HPP
#define PLATFORM_INC_UART_HPP

#include "dma.hpp"
#include "stm32l4xx.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace platform {

/**
 * @brief Describes the result of a UART operation.
 */
enum class UartStatus : std::uint8_t {
    ok,
    busy,
    dma_error
};

/**
 * @brief UART transmitter for an STM32 USART peripheral.
 */
class Uart final {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Constructs a UART wrapper.
     *
     * @param instance USART peripheral instance.
     * @param transmit_dma DMA channel used for transmission.
     */
    constexpr explicit Uart(USART_TypeDef* instance, DmaChannel transmit_dma) noexcept
        : instance_{instance}, transmit_dma_{transmit_dma} {}

    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Transmits one byte.
     *
     * @param byte Byte to transmit.
     * @note this is blocking. use `start_write_dma` for non-blocking transmit
     */
    void write_byte(std::uint8_t byte) noexcept;

    /**
     * @brief Transmits a string.
     *
     * @param text Text to transmit.
     * @note this is blocking. use `start_write_dma` for non-blocking transmit
     */
    void write(std::string_view text) noexcept;

    /**
     * @brief Transmits a string followed by a line ending.
     *
     * @param text Text to transmit.
     * @note this is blocking. use `start_write_dma` for non-blocking transmit
     */
    void write_line(std::string_view text) noexcept;

    /**
     * @brief Starts a UART DMA transmission.
     *
     * This function configures and starts the DMA transfer, then returns
     * immediately without waiting for completion.
     *
     * The transmit buffer must remain valid until the DMA transfer completes.
     *
     * @param data Buffer containing data to transmit.
     * @param size Number of bytes to transmit.
     * @return Status of the operation.
     */
    [[nodiscard]] UartStatus start_write_dma(const std::uint8_t* data, std::size_t size) noexcept;

    /**
     * @brief Transmits a buffer using DMA and waits for completion.
     *
     * This function starts a DMA transfer and blocks until the transfer
     * completes or an error occurs.
     *
     * @param data Buffer containing data to transmit.
     * @param size Number of bytes to transmit.
     * @return Status of the operation.
     */
    [[nodiscard]] UartStatus write_dma(const std::uint8_t* data, std::size_t size) noexcept;

    /**
     * @brief Transmits a string using DMA and waits for completion.
     *
     * @param text Text to transmit.
     * @return Status of the operation.
     */
    [[nodiscard]] UartStatus write_dma(std::string_view text) noexcept;

    /**
     * @brief Transmits a string followed by a line ending using DMA.
     *
     * @param text Text to transmit.
     * @return Status of the operation.
     */
    [[nodiscard]] UartStatus write_line_dma(std::string_view text) noexcept;

    /**
     * @brief Checks whether a DMA transmission is currently active.
     *
     * @return true if a DMA transmission is active; otherwise false.
     */
    [[nodiscard]] bool transmitting_dma() const noexcept;

    /**
     * @brief Handles completion of the UART transmit DMA transfer.
     *
     * Called by the DMA interrupt handler when transmission completes.
     */
    void handle_transmit_dma_complete() noexcept;

    /**
     * @brief Handles an error during a UART transmit DMA transfer.
     *
     * Called by the DMA interrupt handler when a transfer error occurs.
     */
    void handle_transmit_dma_error() noexcept;

  private:
    USART_TypeDef* instance_;
    DmaChannel transmit_dma_;

    std::atomic_bool transmit_active_{false};
    std::atomic_bool transmit_complete_{false};
    std::atomic_bool transmit_error_{false};
}; // class Uart

} // namespace platform

#endif // PLATFORM_INC_UART_HPP
