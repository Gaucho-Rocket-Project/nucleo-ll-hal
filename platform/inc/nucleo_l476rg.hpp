#ifndef PLATFORM_INC_NUCLEO_L476RG_HPP
#define PLATFORM_INC_NUCLEO_L476RG_HPP

#include "digital_input.hpp"
#include "digital_output.hpp"
#include "dma.hpp"
#include "gpio.hpp"
#include "spi.hpp"
#include "stm32l4xx.h"
#include "uart.hpp"
#include <cstdint>

namespace platform::nucleo_l476rg {

/**
 * @brief Selects the board's SPI1 wiring and bus configuration.
 */
enum class Spi1Profile : std::uint8_t {
    generated,    ///< CubeMX settings used by the main application.
    arduino_mode0 ///< PA5/6/7, PB6 CS, Mode 0, 8-bit MSB-first, 625 kHz.
};

/**
 * @brief Initializes the NUCLEO-L476RG platform.
 *
 * Configures the system clock, SysTick time base, GPIO peripherals, status LED,
 * and user-button interrupt.
 *
 * @param spi1_profile SPI1 board configuration. The Arduino profile also
 * configures PB6 as an inactive-high chip-select output and reserves PA5 for
 * SCK, making the status LED unavailable.
 * @note Call this function once before accessing any board devices.
 */
void initialize(Spi1Profile spi1_profile = Spi1Profile::generated) noexcept;

// -------------------------------------------------------------------------
// On-board LED (PA5)

/**
 * @brief Gets the onboard status LED.
 *
 * @return Reference to the digital output controlling the LED.
 */
DigitalOutput& status_led() noexcept;

// -------------------------------------------------------------------------
// On-board Button (PC13)

/**
 * @brief Gets the onboard user button.
 *
 * @return Reference to the digital input representing the user button.
 */
DigitalInput& user_button() noexcept;

// -------------------------------------------------------------------------
// SPI Interface (FIX)

/**
 * @brief Gets the SPI1 master peripheral.
 *
 * @return SPI1 master interface.
 */
Spi& spi1() noexcept;

/**
 * @brief Gets the chip-select output associated with SPI1.
 *
 * @return Active-low chip-select output.
 */
DigitalOutput& spi1_chip_select() noexcept;

// -------------------------------------------------------------------------
// USART2 Interface (PA2, PA3)

/**
 * @brief Gets the UART connected to the ST-LINK virtual COM port.
 *
 * @return Reference to the UART used for console communication.
 */
Uart& console() noexcept;

/**
 * @brief Consumes a pending user-button press event.
 *
 * @return `true` when a button-press event was pending; otherwise `false`.
 *
 * @note A pending event is cleared when this function returns `true`.
 */
[[nodiscard]] bool take_user_button_press() noexcept;

namespace pins {

/**
 * @brief PA4 GPIO pin used as the ADXL345 chip-select signal.
 */
extern const GpioPin pa4;

/**
 * @brief PB6 chip-select pin, configured by the arduino_mode0 SPI1 profile.
 */
extern const GpioPin pb6;

} // namespace pins

namespace detail {

/**
 * @brief Services the user-button EXTI interrupt.
 *
 * Clears the EXTI13 pending flag and records a button-press event for the
 * application.
 *
 * @note Intended to be called only from `EXTI15_10_IRQHandler()`.
 */
void handle_user_button_exti() noexcept;

/**
 * @brief Handles completion of an SPI1 transmit DMA transfer.
 * @note Intended to be called only from `DMA1_Channel3_IRQHandler()`.
 */
void handle_spi1_transmit_dma_complete() noexcept;

/**
 * @brief Handles completion of an SPI1 receive DMA transfer.
 * @note Intended to be called only from `DMA1_Channel2_IRQHandler()`.
 */
void handle_spi1_receive_dma_complete() noexcept;

/**
 * @brief Handles an SPI1 DMA transfer error.
 * @note Intended to be called only from `DMA1_Channel2_IRQHandler()` or `DMA1_Channel3_IRQHandler()`.
 */
void handle_spi1_dma_error() noexcept;

/**
 * @brief Handles completion of the console UART transmit DMA transfer.
 * @note Intended to be called only from `DMA1_Channel7_IRQHandler()`.
 */
void handle_console_transmit_dma_complete() noexcept;

/**
 * @brief Handles an error during the console UART transmit DMA transfer.
 * @note Intended to be called only from `DMA1_Channel7_IRQHandler()`.
 */
void handle_console_transmit_dma_error() noexcept;

} // namespace detail

} // namespace platform::nucleo_l476rg

#endif // PLATFORM_INC_NUCLEO_L476RG_HPP
