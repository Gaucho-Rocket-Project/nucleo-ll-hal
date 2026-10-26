#ifndef BSP_COMMON_MCU_HPP
#define BSP_COMMON_MCU_HPP

#if defined(BSP_MCU_FAMILY_STM32L4) && defined(BSP_MCU_STM32L476XX)
#if !defined(STM32L476xx)
#error "STM32L476xx must be defined for the selected BSP MCU"
#endif

#define BSP_DEVICE_HEADER "stm32l4xx.h"
#define BSP_SYSTEM_HEADER "system_stm32l4xx.h"
#define BSP_HAL_HEADER "stm32l4xx_hal.h"
#define BSP_HAL_CAN_HEADER "stm32l4xx_hal_can.h"
#define BSP_LL_BUS_HEADER "stm32l4xx_ll_bus.h"
#define BSP_LL_DMA_HEADER "stm32l4xx_ll_dma.h"
#define BSP_LL_EXTI_HEADER "stm32l4xx_ll_exti.h"
#define BSP_LL_GPIO_HEADER "stm32l4xx_ll_gpio.h"
#define BSP_LL_PWR_HEADER "stm32l4xx_ll_pwr.h"
#define BSP_LL_RCC_HEADER "stm32l4xx_ll_rcc.h"
#define BSP_LL_SPI_HEADER "stm32l4xx_ll_spi.h"
#define BSP_LL_SYSTEM_HEADER "stm32l4xx_ll_system.h"
#define BSP_LL_USART_HEADER "stm32l4xx_ll_usart.h"
#define BSP_LL_UTILS_HEADER "stm32l4xx_ll_utils.h"

#elif defined(BSP_MCU_FAMILY_STM32F4) && defined(BSP_MCU_STM32F446XX)
#if !defined(STM32F446xx)
#error "STM32F446xx must be defined for the selected BSP MCU"
#endif

#define BSP_DEVICE_HEADER "stm32f4xx.h"
#define BSP_SYSTEM_HEADER "system_stm32f4xx.h"
#define BSP_HAL_HEADER "stm32f4xx_hal.h"
#define BSP_HAL_CAN_HEADER "stm32f4xx_hal_can.h"
#define BSP_LL_BUS_HEADER "stm32f4xx_ll_bus.h"
#define BSP_LL_DMA_HEADER "stm32f4xx_ll_dma.h"
#define BSP_LL_EXTI_HEADER "stm32f4xx_ll_exti.h"
#define BSP_LL_GPIO_HEADER "stm32f4xx_ll_gpio.h"
#define BSP_LL_PWR_HEADER "stm32f4xx_ll_pwr.h"
#define BSP_LL_RCC_HEADER "stm32f4xx_ll_rcc.h"
#define BSP_LL_SPI_HEADER "stm32f4xx_ll_spi.h"
#define BSP_LL_SYSTEM_HEADER "stm32f4xx_ll_system.h"
#define BSP_LL_USART_HEADER "stm32f4xx_ll_usart.h"
#define BSP_LL_UTILS_HEADER "stm32f4xx_ll_utils.h"

#else
#error "Unsupported BSP MCU: select a supported BOARD in CMake"
#endif

#endif // BSP_COMMON_MCU_HPP
