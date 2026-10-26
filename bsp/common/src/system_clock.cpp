#include "mcu.hpp"
#include "system_clock.hpp"

extern "C" {
#include BSP_SYSTEM_HEADER
}

#include BSP_LL_RCC_HEADER
#include BSP_LL_UTILS_HEADER

namespace bsp::clock {

Source source() noexcept {
    switch (LL_RCC_GetSysClkSource()) {
#if defined(BSP_MCU_FAMILY_STM32L4)
    case LL_RCC_SYS_CLKSOURCE_STATUS_MSI:
        return Source::msi;
#endif

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

} // namespace bsp::clock
