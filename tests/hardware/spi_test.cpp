#include "nucleo_l476rg.hpp"
#include "spi_device.hpp"
#include "system_clock.hpp"

#include <array>
#include <cstdint>

namespace {

namespace board = platform::nucleo_l476rg;
using Frame = std::array<std::uint8_t, 4>;
constexpr std::uint32_t transaction_gap_ms{10U};
constexpr std::uint32_t chip_select_hold_us{5U};

void print_frame(const char* label, const Frame& frame) {
    constexpr char hex[]{"0123456789ABCDEF"};
    auto& console = board::console();
    console.write(label);
    for (const auto byte : frame) {
        console.write_byte(hex[byte >> 4U]);
        console.write_byte(hex[byte & 0x0FU]);
        console.write_byte(' ');
    }
    console.write_line("");
}

bool run_test(platform::SpiDevice& slave) {
    // The final OFF clocks out the acknowledgment for OFF sequence 5.
    constexpr std::array<std::uint8_t, 6> commands{1U, 2U, 3U, 4U, 0U, 0U};
    Frame expected{0x5AU, 0x10U, 0U, 0U};
    for (std::size_t index = 0; index < commands.size(); ++index) {
        const Frame tx{0xA5U, commands[index], 0U, static_cast<std::uint8_t>(index + 1U)};
        Frame rx{};
        // SpiDevice holds CS low across all four bytes. Spi::transfer waits
        // for BSY to clear; SpiDevice then adds the configured hold before CS rises.
        const auto status = slave.transfer(tx.data(), rx.data(), tx.size());
        print_frame("TX:     ", tx);
        print_frame("RX:     ", rx);
        print_frame("EXPECT: ", expected);
        if (status != platform::SpiStatus::ok) {
            board::console().write_line(status == platform::SpiStatus::timeout
                                            ? "FAIL: SPI timeout"
                                            : "FAIL: SPI not enabled");
            return false;
        }
        if (rx != expected) {
            board::console().write_line("FAIL: response mismatch (sync/status/previous command/sequence)");
            return false;
        }
        board::console().write_line("OK");
        expected = Frame{0x5AU, 0U, tx[1], tx[3]};
        platform::clock::delay_ms(transaction_gap_ms);
    }
    return true;
}

} // namespace

int main() {
    board::initialize(board::Spi1Profile::arduino_mode0);
    platform::SpiDevice slave{board::spi1(), board::pins::pb6,
                               ActiveLevel::low, chip_select_hold_us};
    slave.initialize();
    auto& console = board::console();
    console.write_line("SPI HIL test: Mode 0, 625 kHz, PA5/PA6/PA7, CS PB6");
    console.write_line("CS hold timing enabled (default 5 us after SPI idle)");

    while (true) {
        console.write_line("Reset ESP32 and wait for slave READY, then press STM32 USER button.");
        static_cast<void>(board::take_user_button_press());
        while (!board::take_user_button_press()) {}
        platform::clock::delay_ms(50U);
        console.write_line(run_test(slave)
                               ? "PASS: READY and all five command acknowledgments verified"
                               : "FAIL: test stopped; check wiring and reset slave before retrying");
    }
}
