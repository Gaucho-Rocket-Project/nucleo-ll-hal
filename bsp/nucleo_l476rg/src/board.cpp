#include "mcu.hpp"
#include "board.hpp"

#include "dma.hpp"
#include "gpio.hpp"
#include "spi.hpp"
#include "system_clock.hpp"
#include "uart.hpp"

extern "C" {
#include "dma.h"
#include "gpio.h"
#include "spi.h"
#include BSP_HAL_HEADER
#include BSP_SYSTEM_HEADER
#include "usart.h"
#include "can.h"
}

#include BSP_LL_BUS_HEADER
#include BSP_LL_EXTI_HEADER
#include BSP_LL_GPIO_HEADER
#include BSP_LL_SPI_HEADER

#include <atomic>

namespace {

platform::GpioOutput status_led_device{GPIOA,
                                       LL_GPIO_PIN_5,
                                       ActiveLevel::high};
platform::GpioInput user_button_device{GPIOC,
                                       LL_GPIO_PIN_13,
                                       ActiveLevel::low};
platform::Spi spi1_device{SPI1,
                          platform::DmaChannel{DMA1, LL_DMA_CHANNEL_3},
                          platform::DmaChannel{DMA1, LL_DMA_CHANNEL_2}};
platform::Uart console_device{USART2,
                              platform::DmaChannel{DMA1, LL_DMA_CHANNEL_7}};

platform::Can can1_device{hcan1};

std::atomic_bool user_button_press_pending{false};

void cube_mx_init() noexcept {
    platform::clock::configure();

    // LL clock setup starts SysTick without its interrupt. Enable the HAL
    // 1 ms time base before CAN initialization can wait on HAL_GetTick().
    // The generated SysTick_Handler already calls HAL_IncTick() once.
    SystemCoreClockUpdate();
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK) {
        Error_Handler();
    }

    /*
     * Required before configuring SYSCFG->EXTICR registers.
     */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART2_UART_Init();
    MX_SPI1_Init();
    LL_SPI_Enable(SPI1);
    MX_CAN1_Init();

    /*
     * Remove stale pending state before enabling the interrupt.
     */
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);

    NVIC_SetPriority(EXTI15_10_IRQn, 0U);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

} // namespace

namespace platform::board::pins {

const GpioPin pa4{GPIOA, LL_GPIO_PIN_4};

} // namespace platform::board::pins

namespace platform::board {

void initialize() noexcept {
    // CubeMX-generated clock initialization.
    cube_mx_init();

    // Start with the LED turned off.
    status_led_device.clear();
}

DigitalOutput& status_led() noexcept {
    return status_led_device;
}

DigitalInput& user_button() noexcept {
    return user_button_device;
}

Uart& console() noexcept {
    return console_device;
}

Spi& spi1() noexcept {
    return spi1_device;
}

Can& can1() noexcept {
    return can1_device;
}

bool take_user_button_press() noexcept {
    return user_button_press_pending.exchange(
        false,
        std::memory_order_relaxed);
}

namespace detail {

void handle_spi1_transmit_dma_complete() noexcept {
    spi1_device.handle_transmit_dma_complete();
}

void handle_spi1_receive_dma_complete() noexcept {
    spi1_device.handle_receive_dma_complete();
}

void handle_spi1_dma_error() noexcept {
    spi1_device.handle_dma_error();
}

void handle_console_transmit_dma_complete() noexcept {
    console_device.handle_transmit_dma_complete();
}

void handle_console_transmit_dma_error() noexcept {
    console_device.handle_transmit_dma_error();
}

void handle_user_button_exti() noexcept {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != 0U) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        user_button_press_pending.store(
            true,
            std::memory_order_relaxed);
    }
}

} // namespace detail

} // namespace platform::board
