#ifndef PLATFORM_INC_DMA_HPP
#define PLATFORM_INC_DMA_HPP

#include "mcu.hpp"
#include BSP_DEVICE_HEADER

#include <cstdint>

namespace bsp {

/**
 * @brief Identifies a hardware DMA channel (L4) or stream (F4).
 */
struct DmaChannel {
    DMA_TypeDef* controller;
    // LL_DMA_CHANNEL_* on L4; LL_DMA_STREAM_* on F4. CubeMX configures
    // the F4 request-channel selection separately from this stream index.
    std::uint32_t channel;

    void disable() const noexcept;
    void enable() const noexcept;
    void clear_flags() const noexcept;
    void enable_interrupts() const noexcept;
};

} // namespace bsp

#endif // PLATFORM_INC_DMA_HPP