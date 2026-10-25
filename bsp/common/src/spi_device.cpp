#include "spi_device.hpp"
#include "system_clock.hpp"

namespace {

// Keep the device selected until the transfer and its hold time have finished,
// including early returns on errors. No hold is inserted between transfer phases.
class ChipSelectGuard final {
  public:
    ChipSelectGuard(platform::GpioOutput& chip_select, std::uint32_t hold_us) noexcept
        : chip_select_{chip_select}, hold_us_{hold_us} {
        chip_select_.set();
    }

    ~ChipSelectGuard() {
        if (hold_us_ != 0U) platform::clock::delay_us(hold_us_);
        chip_select_.clear();
    }

    ChipSelectGuard(const ChipSelectGuard&) = delete;
    ChipSelectGuard& operator=(const ChipSelectGuard&) = delete;

  private:
    platform::GpioOutput& chip_select_;
    std::uint32_t hold_us_;
};

} // namespace

namespace platform {

SpiDevice::SpiDevice(Spi& spi, GpioPin chip_select_pin, ::ActiveLevel active_level, std::uint32_t chip_select_hold_us) noexcept
    : spi_{spi}, chip_select_{chip_select_pin, active_level}, chip_select_hold_us_{chip_select_hold_us} {}

void SpiDevice::initialize() noexcept {
    chip_select_.clear();
}

SpiStatus SpiDevice::transfer(const std::uint8_t* transmit, std::uint8_t* receive, std::size_t size) noexcept {
    ChipSelectGuard chip_select_guard{chip_select_, chip_select_hold_us_};
    return spi_.transfer(transmit, receive, size);
}

SpiStatus SpiDevice::transfer_dma(const std::uint8_t* transmit, std::uint8_t* receive, std::size_t size) noexcept {
    ChipSelectGuard chip_select_guard{chip_select_, chip_select_hold_us_};
    return spi_.transfer_dma(transmit, receive, size);
}

SpiStatus SpiDevice::write_then_read(const std::uint8_t* command,
                                     std::size_t command_size,
                                     std::uint8_t* receive,
                                     std::size_t receive_size) noexcept {
    ChipSelectGuard chip_select_guard{chip_select_, chip_select_hold_us_};
    const SpiStatus status{spi_.transfer(command, nullptr, command_size)};
    if (status != SpiStatus::ok) return status;
    return spi_.transfer(nullptr, receive, receive_size);
}

SpiStatus SpiDevice::write_then_read_dma(const std::uint8_t* command,
                                         std::size_t command_size,
                                         std::uint8_t* receive,
                                         std::size_t receive_size) noexcept {
    ChipSelectGuard chip_select_guard{chip_select_, chip_select_hold_us_};
    const SpiStatus status{spi_.transfer_dma(command, nullptr, command_size)};
    if (status != SpiStatus::ok) return status;
    return spi_.transfer_dma(nullptr, receive, receive_size);
}

} // namespace platform