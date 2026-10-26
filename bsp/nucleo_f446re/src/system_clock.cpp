#include "mcu.hpp"
#include "system_clock.hpp"

#include BSP_LL_BUS_HEADER
#include BSP_LL_PWR_HEADER
#include BSP_LL_RCC_HEADER
#include BSP_LL_SYSTEM_HEADER
#include BSP_LL_UTILS_HEADER

namespace bsp::clock {

// Matches mcal/stm32f4/Core/Src/main.c: HSI PLL, 84 MHz HCLK,
// 42 MHz APB1 and 84 MHz APB2.
void configure() noexcept {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {}
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
    LL_PWR_DisableOverDriveMode();
    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) {}

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_16,
                               336, LL_RCC_PLLP_DIV_4);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1) {}
    while (LL_PWR_IsActiveFlag_VOS() == 0) {}

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}

    LL_SetSystemCoreClock(84000000);
    LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}

} // namespace bsp::clock
