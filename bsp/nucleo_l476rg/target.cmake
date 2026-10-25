set(BSP_MCU stm32l476rg)
set(MCAL_DIR "${PROJECT_SOURCE_DIR}/mcal/stm32l4")
set(MCAL_DRIVER_DIR "${MCAL_DIR}/Drivers/STM32L4xx_HAL_Driver")
set(MCAL_STARTUP_SOURCE "${MCAL_DIR}/startup_stm32l476xx.s")
set(GENERATED_LINKER_SCRIPT "${MCAL_DIR}/STM32L476xx_FLASH.ld")
set(MCU_ARCH_OPTIONS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

# ---------------------------------------------------------------------------
# NUCLEO-L476RG configuration shared by firmware and BSP targets.
# ---------------------------------------------------------------------------

add_library(mcal_config INTERFACE)

target_include_directories(mcal_config INTERFACE
    "${MCAL_DIR}/Core/Inc"
)

# Treat third-party ST headers as system headers so warnings inside CMSIS
# and the LL headers do not overwhelm your C++ build output.
target_include_directories(mcal_config SYSTEM INTERFACE
    "${MCAL_DIR}/Drivers/CMSIS/Include"
    "${MCAL_DIR}/Drivers/CMSIS/Device/ST/STM32L4xx/Include"
    "${MCAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Inc"
)

target_compile_definitions(mcal_config INTERFACE
    BSP_BOARD_NUCLEO_L476RG=1
    BSP_MCU_STM32L476XX=1
    BSP_MCU_FAMILY_STM32L4=1
    STM32L476xx
    USE_FULL_LL_DRIVER

    HSE_VALUE=8000000
    HSI_VALUE=16000000
    LSE_VALUE=32768
    LSI_VALUE=32000
    MSI_VALUE=4000000

    HSE_STARTUP_TIMEOUT=100
    LSE_STARTUP_TIMEOUT=5000

    EXTERNALSAI1_CLOCK_VALUE=2097000
    EXTERNALSAI2_CLOCK_VALUE=2097000

    VDD_VALUE=3300
    PREFETCH_ENABLE=1
    INSTRUCTION_CACHE_ENABLE=1
    DATA_CACHE_ENABLE=1
)

target_compile_options(mcal_config INTERFACE
    ${MCU_ARCH_OPTIONS}

    -ffunction-sections
    -fdata-sections
    -fstack-usage
)

