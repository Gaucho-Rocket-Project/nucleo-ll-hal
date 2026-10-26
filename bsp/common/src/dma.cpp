#include "dma.hpp"
#include BSP_LL_DMA_HEADER

namespace bsp {

void DmaChannel::disable() const noexcept {
#if defined(BSP_MCU_FAMILY_STM32F4)
    LL_DMA_DisableStream(controller, channel);
    // F4 may finish an in-flight bus transaction before clearing EN.
    // Configuration registers and transfer buffers are safe only afterward.
    while (LL_DMA_IsEnabledStream(controller, channel) != 0U) {}
#else
    LL_DMA_DisableChannel(controller, channel);
#endif
}

void DmaChannel::enable() const noexcept {
#if defined(BSP_MCU_FAMILY_STM32F4)
    LL_DMA_EnableStream(controller, channel);
#else
    LL_DMA_EnableChannel(controller, channel);
#endif
}

void DmaChannel::enable_interrupts() const noexcept {
    LL_DMA_EnableIT_TC(controller, channel);
    LL_DMA_EnableIT_TE(controller, channel);
#if defined(BSP_MCU_FAMILY_STM32F4)
    LL_DMA_EnableIT_DME(controller, channel);
    LL_DMA_EnableIT_FE(controller, channel);
#endif
}

void DmaChannel::clear_flags() const noexcept {
#if defined(BSP_MCU_FAMILY_STM32F4)
    switch (channel) {
    case LL_DMA_STREAM_0:
        LL_DMA_ClearFlag_TC0(controller);
        LL_DMA_ClearFlag_HT0(controller);
        LL_DMA_ClearFlag_TE0(controller);
        LL_DMA_ClearFlag_DME0(controller);
        LL_DMA_ClearFlag_FE0(controller);
        break;
    case LL_DMA_STREAM_1:
        LL_DMA_ClearFlag_TC1(controller);
        LL_DMA_ClearFlag_HT1(controller);
        LL_DMA_ClearFlag_TE1(controller);
        LL_DMA_ClearFlag_DME1(controller);
        LL_DMA_ClearFlag_FE1(controller);
        break;
    case LL_DMA_STREAM_2:
        LL_DMA_ClearFlag_TC2(controller);
        LL_DMA_ClearFlag_HT2(controller);
        LL_DMA_ClearFlag_TE2(controller);
        LL_DMA_ClearFlag_DME2(controller);
        LL_DMA_ClearFlag_FE2(controller);
        break;
    case LL_DMA_STREAM_3:
        LL_DMA_ClearFlag_TC3(controller);
        LL_DMA_ClearFlag_HT3(controller);
        LL_DMA_ClearFlag_TE3(controller);
        LL_DMA_ClearFlag_DME3(controller);
        LL_DMA_ClearFlag_FE3(controller);
        break;
    case LL_DMA_STREAM_4:
        LL_DMA_ClearFlag_TC4(controller);
        LL_DMA_ClearFlag_HT4(controller);
        LL_DMA_ClearFlag_TE4(controller);
        LL_DMA_ClearFlag_DME4(controller);
        LL_DMA_ClearFlag_FE4(controller);
        break;
    case LL_DMA_STREAM_5:
        LL_DMA_ClearFlag_TC5(controller);
        LL_DMA_ClearFlag_HT5(controller);
        LL_DMA_ClearFlag_TE5(controller);
        LL_DMA_ClearFlag_DME5(controller);
        LL_DMA_ClearFlag_FE5(controller);
        break;
    case LL_DMA_STREAM_6:
        LL_DMA_ClearFlag_TC6(controller);
        LL_DMA_ClearFlag_HT6(controller);
        LL_DMA_ClearFlag_TE6(controller);
        LL_DMA_ClearFlag_DME6(controller);
        LL_DMA_ClearFlag_FE6(controller);
        break;
    case LL_DMA_STREAM_7:
        LL_DMA_ClearFlag_TC7(controller);
        LL_DMA_ClearFlag_HT7(controller);
        LL_DMA_ClearFlag_TE7(controller);
        LL_DMA_ClearFlag_DME7(controller);
        LL_DMA_ClearFlag_FE7(controller);
        break;
    }
#else
    switch (channel) {
    case LL_DMA_CHANNEL_1: LL_DMA_ClearFlag_GI1(controller); break;
    case LL_DMA_CHANNEL_2: LL_DMA_ClearFlag_GI2(controller); break;
    case LL_DMA_CHANNEL_3: LL_DMA_ClearFlag_GI3(controller); break;
    case LL_DMA_CHANNEL_4: LL_DMA_ClearFlag_GI4(controller); break;
    case LL_DMA_CHANNEL_5: LL_DMA_ClearFlag_GI5(controller); break;
    case LL_DMA_CHANNEL_6: LL_DMA_ClearFlag_GI6(controller); break;
    case LL_DMA_CHANNEL_7: LL_DMA_ClearFlag_GI7(controller); break;
    }
#endif
}

} // namespace bsp
