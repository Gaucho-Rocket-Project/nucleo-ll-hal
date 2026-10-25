extern "C" {
#include "mcu.hpp"
#include "main.h"
}

#include BSP_DEVICE_HEADER

extern "C" void Error_Handler(void) {
    __disable_irq();

    while (true) {
        __NOP();
    }
}