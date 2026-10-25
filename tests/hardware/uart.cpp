#include "nucleo_l476rg.hpp"
#include "system_clock.hpp"

// Observe one complete line per second on the ST-LINK virtual COM port.
int main() {
    namespace board = platform::nucleo_l476rg;
    board::initialize();

    while (true) {
        board::console().write_line("UART polling smoke test: 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        board::status_led().toggle();
        platform::clock::delay_ms(1000U);
    }
}
