set(BSP_MCU stm32f446re)
set(MCAL_DIR "${PROJECT_SOURCE_DIR}/mcal/stm32f4")
set(MCAL_DRIVER_DIR "${MCAL_DIR}/Drivers/STM32F4xx_HAL_Driver")
set(MCAL_STARTUP_SOURCE "${MCAL_DIR}/startup_stm32f446xx.s")
set(GENERATED_LINKER_SCRIPT "${MCAL_DIR}/STM32F446xx_FLASH.ld")
set(MCU_ARCH_OPTIONS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

# ---------------------------------------------------------------------------
# NUCLEO-F446RE configuration shared by firmware and BSP targets.
# ---------------------------------------------------------------------------

add_library(mcal_config INTERFACE)

target_include_directories(mcal_config INTERFACE
    "${MCAL_DIR}/Core/Inc"
)

# Treat third-party ST headers as system headers so warnings inside CMSIS
# and the LL headers do not overwhelm your C++ build output.
target_include_directories(mcal_config SYSTEM INTERFACE
    "${MCAL_DIR}/Drivers/CMSIS/Include"
    "${MCAL_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include"
    "${MCAL_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc"
)

target_compile_definitions(mcal_config INTERFACE
    BSP_BOARD_NUCLEO_F446RE=1
    BSP_MCU_STM32F446XX=1
    BSP_MCU_FAMILY_STM32F4=1
    STM32F446xx
    USE_FULL_LL_DRIVER

    HSE_VALUE=8000000
    HSI_VALUE=16000000
    LSE_VALUE=32768
    LSI_VALUE=32000

    HSE_STARTUP_TIMEOUT=100
    LSE_STARTUP_TIMEOUT=5000

    EXTERNAL_CLOCK_VALUE=12288000

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

# Retain generated exception/SysTick handlers, but give its peripheral stubs
# private names so BSP handlers own these vectors without editing MCAL.
set_source_files_properties("${MCAL_DIR}/Core/Src/stm32f4xx_it.c" PROPERTIES
    COMPILE_DEFINITIONS
        "DMA1_Stream6_IRQHandler=CubeMX_DMA1_Stream6_IRQHandler;DMA2_Stream0_IRQHandler=CubeMX_DMA2_Stream0_IRQHandler;DMA2_Stream3_IRQHandler=CubeMX_DMA2_Stream3_IRQHandler;EXTI15_10_IRQHandler=CubeMX_EXTI15_10_IRQHandler"
)
