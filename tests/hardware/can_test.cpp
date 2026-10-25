
#include "can.hpp"
#include "board.hpp"
#include "system_clock.hpp"

int main() {
    namespace board = platform::board;

    board::initialize();

    auto& console = board::console();
    auto& can = board::can1();

    console.write_line("CAN1 classic loopback test");

    if (can.initialize() != platform::CanStatus::ok) {
        console.write_line("FAIL: CAN filter/start");
        while (true) {}
    }

    platform::CanFrame tx{};
    tx.id = 0x123;
    tx.length = 2;
    tx.data[0] = 0xAB;
    tx.data[1] = 0xCD;

    while (true) {
        if (can.send(tx) != platform::CanStatus::ok) {
            console.write_line("FAIL: CAN transmit enqueue");
        } else {
            bool received = false;

            for (unsigned i = 0; i < 100U; ++i) {
                platform::CanFrame rx{};
                const auto status = can.receive(rx);

                if (status == platform::CanStatus::ok) {
                    received =
                        rx.id == 0x123U &&
                        rx.length == 2U &&
                        rx.data[0] == 0xABU &&
                        rx.data[1] == 0xCDU;
                    break;
                }

                if (status != platform::CanStatus::empty) {
                    break;
                }

                platform::clock::delay_ms(1U);
            }

            console.write_line(
                received ? "PASS: 0x123 AB CD"
                         : "FAIL: no matching loopback frame"
            );
        }

        platform::clock::delay_ms(1000U);
    }
}
