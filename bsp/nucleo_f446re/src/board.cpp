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

bsp::GpioOutput status_led_device{GPIOA,
                                       LL_GPIO_PIN_5,
                                       ActiveLevel::high};
bsp::GpioInput user_button_device{GPIOC,
                                       LL_GPIO_PIN_13,
                                       ActiveLevel::low};
bsp::Spi spi1_device{SPI1,
                          bsp::DmaChannel{DMA2, LL_DMA_STREAM_3},
                          bsp::DmaChannel{DMA2, LL_DMA_STREAM_0}};
bsp::Uart console_device{USART2,
                              bsp::DmaChannel{DMA1, LL_DMA_STREAM_6}};

bsp::Can can1_device{hcan1};

std::atomic_bool user_button_press_pending{false};

void cube_mx_init() noexcept {
    bsp::clock::configure();

    // Enable the HAL
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
    // CubeMX does not configure a chip select for its software-NSS SPI1.
    // PA4 is the common board API's active-low device select.
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_4, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_4, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
    MX_DMA_Init();
    MX_USART2_UART_Init();
    MX_SPI1_Init();
    // Keep the shared ADXL345 application in Mode 3; CubeMX defaults to Mode 0.
    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);
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

namespace bsp::board::pins {

const GpioPin pa4{GPIOA, LL_GPIO_PIN_4};

} // namespace bsp::board::pins

namespace bsp::board {

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

} // namespace bsp::board
