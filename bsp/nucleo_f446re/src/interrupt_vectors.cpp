#include "board.hpp"
#include BSP_LL_DMA_HEADER

extern "C" void EXTI15_10_IRQHandler() {
    bsp::board::detail::handle_user_button_exti();
}

extern "C" void DMA2_Stream0_IRQHandler() {
    // Error takes precedence when completion and error arrive together.
    if (LL_DMA_IsActiveFlag_TE0(DMA2) != 0U ||
        LL_DMA_IsActiveFlag_DME0(DMA2) != 0U ||
        LL_DMA_IsActiveFlag_FE0(DMA2) != 0U) {
        bsp::DmaChannel{DMA2, LL_DMA_STREAM_0}.clear_flags();
        bsp::board::detail::handle_spi1_dma_error();
    } else if (LL_DMA_IsActiveFlag_TC0(DMA2) != 0U) {
        LL_DMA_ClearFlag_TC0(DMA2);
        bsp::board::detail::handle_spi1_receive_dma_complete();
    }
}

extern "C" void DMA2_Stream3_IRQHandler() {
    // Error takes precedence when completion and error arrive together.
    if (LL_DMA_IsActiveFlag_TE3(DMA2) != 0U ||
        LL_DMA_IsActiveFlag_DME3(DMA2) != 0U ||
        LL_DMA_IsActiveFlag_FE3(DMA2) != 0U) {
        bsp::DmaChannel{DMA2, LL_DMA_STREAM_3}.clear_flags();
        bsp::board::detail::handle_spi1_dma_error();
    } else if (LL_DMA_IsActiveFlag_TC3(DMA2) != 0U) {
        LL_DMA_ClearFlag_TC3(DMA2);
        bsp::board::detail::handle_spi1_transmit_dma_complete();
    }
}

extern "C" void DMA1_Stream6_IRQHandler() {
    // Error takes precedence when completion and error arrive together.
    if (LL_DMA_IsActiveFlag_TE6(DMA1) != 0U ||
        LL_DMA_IsActiveFlag_DME6(DMA1) != 0U ||
        LL_DMA_IsActiveFlag_FE6(DMA1) != 0U) {
        bsp::DmaChannel{DMA1, LL_DMA_STREAM_6}.clear_flags();
        bsp::board::detail::handle_console_transmit_dma_error();
    } else if (LL_DMA_IsActiveFlag_TC6(DMA1) != 0U) {
        LL_DMA_ClearFlag_TC6(DMA1);
        bsp::board::detail::handle_console_transmit_dma_complete();
    }
}

