#include "nucleo_l476rg.hpp"
#include "system_clock.hpp"

// Exercises DMA transmission and its completion interrupt repeatedly.
// A missing completion interrupt leaves the LED stopped inside write_line_dma.
int main() {
    namespace board = platform::nucleo_l476rg;
    board::initialize();
    auto& console = board::console();
    console.write_line("Starting UART DMA smoke test");

    while (true) {
        const auto status = console.write_line_dma(
            "UART DMA smoke test: 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        if (status != platform::UartStatus::ok) {
            console.write_line("FAIL: UART DMA transfer");
            board::status_led().set();
            while (true); // Stop here so that the initialization error remains visible.
        }
        board::status_led().toggle();
        platform::clock::delay_ms(1000U);
    }
}
