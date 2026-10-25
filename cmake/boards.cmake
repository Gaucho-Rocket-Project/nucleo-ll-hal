set(BOARD "NUCLEO_L476RG" CACHE STRING "Target STM32 board")
set_property(CACHE BOARD PROPERTY STRINGS NUCLEO_L476RG)

if(BOARD STREQUAL "NUCLEO_L476RG")
    set(BSP_BOARD nucleo_l476rg)
elseif(BOARD STREQUAL "NUCLEO_F446RE")
    message(FATAL_ERROR
        "NUCLEO_F446RE support is not implemented yet. Use BOARD=NUCLEO_L476RG.")
else()
    message(FATAL_ERROR "Unsupported BOARD '${BOARD}'. Supported: NUCLEO_L476RG")
endif()

set(BSP_BOARD_DIR "${PROJECT_SOURCE_DIR}/bsp/${BSP_BOARD}")
