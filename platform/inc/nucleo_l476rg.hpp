#ifndef PLATFORM_INC_NUCLEO_L476RG_HPP
#define PLATFORM_INC_NUCLEO_L476RG_HPP

#include "digital_input.hpp"
#include "digital_output.hpp"
#include "dma.hpp"
#include "gpio.hpp"
#include "spi.hpp"
#include "can.hpp"
#include "uart.hpp"
#include "stm32l4xx.h"
#include <cstdint>

namespace platform::nucleo_l476rg {

/**
 * @brief Initializes the NUCLEO-L476RG platform.
 *
 * Configures the system clock, SysTick time base, GPIO peripherals, status LED,
 * and user-button interrupt.
 *
 * @note Call this function once before accessing any board devices.
 */
void initialize() noexcept;

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
// SPI1 Interface (PB3 SCK, PB4 MISO, PB5 MOSI)

/**
 * @brief Gets the SPI1 master peripheral.
 *
 * @return SPI1 master interface.
 */
Spi& spi1() noexcept;

// -------------------------------------------------------------------------
// CAN Interface (PB8, PB9)

/**
 * @brief Gets the CAN1 master peripheral.
 *
 * @return CAN1 master interface.
 */
Can& can1() noexcept;

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
 * @brief PA4 GPIO pin used as the SPI1 device chip-select signal.
 */
extern const GpioPin pa4;

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
