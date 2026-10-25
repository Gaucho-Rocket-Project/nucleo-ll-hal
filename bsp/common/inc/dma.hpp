#ifndef PLATFORM_INC_DMA_HPP
#define PLATFORM_INC_DMA_HPP

#include "mcu.hpp"
#include BSP_DEVICE_HEADER

#include <cstdint>

namespace platform {

/**
 * @brief Identifies a hardware DMA channel.
 */
struct DmaChannel {
    DMA_TypeDef* controller;
    std::uint32_t channel;
};

} // namespace platform

#endif // PLATFORM_INC_DMA_HPP