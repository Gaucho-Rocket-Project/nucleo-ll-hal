
#include "can.hpp"
#include "board.hpp"
#include "system_clock.hpp"

int main() {
    namespace board = bsp::board;

    board::initialize();

    auto& console = board::console();
    auto& can = board::can1();

    console.write_line("CAN1 classic loopback test");

    if (can.initialize() != bsp::CanStatus::ok) {
        console.write_line("FAIL: CAN filter/start");
        while (true) {}
    }

    bsp::CanFrame tx{};
    tx.id = 0x123;
    tx.length = 2;
    tx.data[0] = 0xAB;
    tx.data[1] = 0xCD;

    while (true) {
        if (can.send(tx) != bsp::CanStatus::ok) {
            console.write_line("FAIL: CAN transmit enqueue");
        } else {
            bool received = false;

            for (unsigned i = 0; i < 100U; ++i) {
                bsp::CanFrame rx{};
                const auto status = can.receive(rx);

                if (status == bsp::CanStatus::ok) {
                    received =
                        rx.id == 0x123U &&
                        rx.length == 2U &&
                        rx.data[0] == 0xABU &&
                        rx.data[1] == 0xCDU;
                    break;
                }

                if (status != bsp::CanStatus::empty) {
                    break;
                }

                bsp::clock::delay_ms(1U);
            }

            console.write_line(
                received ? "PASS: 0x123 AB CD"
                         : "FAIL: no matching loopback frame"
            );
        }

        bsp::clock::delay_ms(1000U);
    }
}
