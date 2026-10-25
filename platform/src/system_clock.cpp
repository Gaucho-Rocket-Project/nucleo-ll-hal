#include "system_clock.hpp"

extern "C" {
#include "system_stm32l4xx.h"
}

#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"

namespace platform::clock {

void configure() noexcept {
    /*
     * Copied CubeMX's generated
     * SystemClock_Config() function here.
     *
     * Source:
     * mcal/Core/Src/main.c
     */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    while (LL_PWR_IsActiveFlag_VOS() != 0);
    LL_RCC_HSI_Enable();

    /* Wait till HSI is ready */
    while (LL_RCC_HSI_IsReady() != 1);
    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 10, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_Init1msTick(80000000);

    LL_SetSystemCoreClock(80000000);
}

Source source() noexcept {
    switch (LL_RCC_GetSysClkSource()) {
    case LL_RCC_SYS_CLKSOURCE_STATUS_MSI:
        return Source::msi;

    case LL_RCC_SYS_CLKSOURCE_STATUS_HSI:
        return Source::hsi16;

    case LL_RCC_SYS_CLKSOURCE_STATUS_HSE:
        return Source::hse;

    case LL_RCC_SYS_CLKSOURCE_STATUS_PLL:
        return Source::pll;

    default:
        return Source::unknown;
    }
}

std::uint32_t frequency_hz() noexcept {
    SystemCoreClockUpdate();
    return SystemCoreClock;
}

void delay_ms(std::uint32_t milliseconds) noexcept {
    if (milliseconds == 0U) return;
    LL_mDelay(milliseconds);
}

void delay_us(std::uint32_t microseconds) noexcept {
    if (microseconds == 0U) return;

    CoreDebug->DEMCR = CoreDebug->DEMCR | CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL = DWT->CTRL | DWT_CTRL_CYCCNTENA_Msk;
    __DSB();
    __ISB();

    const std::uint32_t cycles_per_us{(SystemCoreClock + 999999U) / 1000000U};

    while (microseconds != 0U) {
        const std::uint32_t start{DWT->CYCCNT};
        while (static_cast<std::uint32_t>(DWT->CYCCNT - start) < cycles_per_us);
        --microseconds;
    }
}

} // namespace platform::clock
