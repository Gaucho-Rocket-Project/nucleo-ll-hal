#include "nucleo_l476rg.hpp"

#include "dma.hpp"
#include "gpio.hpp"
#include "spi.hpp"
#include "system_clock.hpp"
#include "uart.hpp"

extern "C" {
#include "dma.h"
#include "gpio.h"
#include "spi.h"
#include "system_stm32l4xx.h"
#include "usart.h"
}

#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_utils.h"

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

std::atomic_bool user_button_press_pending{false};

void configure_arduino_spi1() noexcept {
    LL_SPI_Disable(SPI1);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA |
                             LL_AHB2_GRP1_PERIPH_GPIOB);

    // Release the application's SPI pin mapping before selecting PA5/6/7.
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_ANALOG);

    // Preload CS high before enabling its output driver.
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6);
    LL_GPIO_InitTypeDef pins{};
    pins.Pin = LL_GPIO_PIN_6;
    pins.Mode = LL_GPIO_MODE_OUTPUT;
    pins.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    pins.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    pins.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &pins);

    pins.Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
    pins.Mode = LL_GPIO_MODE_ALTERNATE;
    pins.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(GPIOA, &pins);

    LL_SPI_InitTypeDef spi{};
    spi.TransferDirection = LL_SPI_FULL_DUPLEX;
    spi.Mode = LL_SPI_MODE_MASTER;
    spi.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    spi.ClockPolarity = LL_SPI_POLARITY_LOW;
    spi.ClockPhase = LL_SPI_PHASE_1EDGE;
    spi.NSS = LL_SPI_NSS_SOFT;
    spi.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV128; // 80 MHz / 128 = 625 kHz.
    spi.BitOrder = LL_SPI_MSB_FIRST;
    spi.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    spi.CRCPoly = 7U;
    LL_SPI_Init(SPI1, &spi);
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_DisableNSSPulseMgt(SPI1);
    LL_SPI_SetRxFIFOThreshold(SPI1, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_Enable(SPI1);
}

void cube_mx_init() noexcept {
    platform::clock::configure();

    /*
     * Required before configuring SYSCFG->EXTICR registers.
     */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART2_UART_Init();
    MX_SPI1_Init();
    LL_SPI_Enable(SPI1);

    SystemCoreClockUpdate();
    LL_Init1msTick(SystemCoreClock);

    /*
     * Remove stale pending state before enabling the interrupt.
     */
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);

    NVIC_SetPriority(EXTI15_10_IRQn, 0U);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

} // namespace

namespace platform::nucleo_l476rg::pins {

const GpioPin pa4{GPIOA, LL_GPIO_PIN_4};
const GpioPin pb6{GPIOB, LL_GPIO_PIN_6};

} // namespace platform::nucleo_l476rg::pins

namespace platform::nucleo_l476rg {

void initialize(Spi1Profile spi1_profile) noexcept {
    // CubeMX-generated clock initialization.
    cube_mx_init();

    // Start with the LED turned off.
    status_led_device.clear();

    if (spi1_profile == Spi1Profile::arduino_mode0) {
        configure_arduino_spi1();
    }
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

} // namespace platform::nucleo_l476rg
