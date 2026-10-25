#include "adxl345.hpp"

namespace application::sensors {

Adxl345::Adxl345(platform::Spi& spi, platform::GpioPin chip_select_pin) noexcept
    : device_{spi, chip_select_pin, ::ActiveLevel::low} {}

SensorStatus Adxl345::initialize() noexcept {
    this->initialized_ = false;
    device_.initialize();
    std::uint8_t device_id = 0U;
    // Get Device ID
    SensorStatus status = read_register(Register::device_id, device_id);
    if (status != SensorStatus::ok) return status;
    if (device_id != expected_device_id) return SensorStatus::unexpected_device;
    // Configure the device while it is in standby mode.
    status = write_register(Register::power_control, standby_mode);
    if (status != SensorStatus::ok) return status;
    // Set data format.
    status = write_register(Register::data_format, data_format_configuration);
    if (status != SensorStatus::ok) return status;
    // Set bandwidth rate.
    status = write_register(Register::bandwidth_rate, bandwidth_configuration);
    if (status != SensorStatus::ok) return status;
    // Enter measurement mode.
    status = write_register(Register::power_control, measurement_mode);
    if (status != SensorStatus::ok) return status;
    // Verify that measurement mode was accepted.
    std::uint8_t power_control = 0U;
    // Set Power Control.
    status = read_register(Register::power_control, power_control);
    if (status != SensorStatus::ok) return status;
    if ((power_control & measurement_mode) == 0U) return SensorStatus::configuration_error;

    this->initialized_ = true;
    return SensorStatus::ok;
}

SensorStatus Adxl345::read(AccelerationSample& sample) noexcept {
    if (!initialized_) return SensorStatus::not_initialized;

    std::uint8_t data[6]{};
    const SensorStatus status{read_registers(Register::data_x0, data, sizeof(data))};
    if (status != SensorStatus::ok) return status;

    sample.x = combine_bytes(data[0], data[1]);
    sample.y = combine_bytes(data[2], data[3]);
    sample.z = combine_bytes(data[4], data[5]);

    return SensorStatus::ok;
}

bool Adxl345::initialized() const noexcept {
    return initialized_;
}

SensorStatus Adxl345::read_register(Register register_address, std::uint8_t& value) noexcept {
    return read_registers(register_address, &value, 1U);
}

SensorStatus Adxl345::read_registers(Register start_address, std::uint8_t* values, std::size_t count) noexcept {
    if ((values == nullptr) || (count == 0U)) return SensorStatus::communication_error;
    std::uint8_t command{static_cast<std::uint8_t>(static_cast<std::uint8_t>(start_address) | read_command)};
    if (count > 1U) command |= multibyte_command;
    return convert_status(device_.write_then_read(&command, 1U, values, count));
}

SensorStatus Adxl345::write_register(Register register_address, std::uint8_t value) noexcept {
    const std::uint8_t transmission[]{static_cast<std::uint8_t>(register_address), value};
    return convert_status(device_.transfer(transmission, nullptr, sizeof(transmission)));
}

SensorStatus Adxl345::convert_status(platform::SpiStatus status) noexcept {
    if (status == platform::SpiStatus::ok) return SensorStatus::ok;
    return SensorStatus::communication_error;
}

std::int16_t Adxl345::combine_bytes(std::uint8_t low, std::uint8_t high) noexcept {
    const std::uint16_t unsigned_value{static_cast<std::uint16_t>((static_cast<std::uint16_t>(high) << 8U) |
                                                                  static_cast<std::uint16_t>(low))};
    /*
     * Bit 15 is the sign bit of the two's-complement acceleration value.
     *
     * When the sign bit is set, subtract 2^16 to convert the unsigned
     * representation into the equivalent negative signed value.
     */
    if ((unsigned_value & 0x8000U) != 0U) {
        return static_cast<std::int16_t>(static_cast<std::int32_t>(unsigned_value) - 0x10000);
    }
    return static_cast<std::int16_t>(unsigned_value);
}

/**
 * @brief Appends one character to a fixed-size buffer.
 *
 * @param buffer Destination buffer.
 * @param capacity Capacity of the destination buffer.
 * @param position Current write position.
 * @param character Character to append.
 */
void append_character(char* buffer, std::size_t capacity, std::size_t& position, char character) noexcept {
    if ((buffer != nullptr) && (position < capacity)) {
        buffer[position] = character;
        ++position;
    }
}

/**
 * @brief Appends text to a fixed-size buffer.
 *
 * @param buffer Destination buffer.
 * @param capacity Capacity of the destination buffer.
 * @param position Current write position.
 * @param text Text to append.
 */
void append_text(char* buffer, std::size_t capacity, std::size_t& position, std::string_view text) noexcept {
    for (const char character : text) {
        append_character(buffer, capacity, position, character);
    }
}

/**
 * @brief Appends a signed 16-bit integer to a fixed-size buffer.
 *
 * @param buffer Destination buffer.
 * @param capacity Capacity of the destination buffer.
 * @param position Current write position.
 * @param value Integer to append.
 */
void append_integer(char* buffer, std::size_t capacity, std::size_t& position, std::int16_t value) noexcept {
    std::int32_t signed_value{static_cast<std::int32_t>(value)};
    if (signed_value < 0) {
        append_character(buffer, capacity, position, '-');
        signed_value = -signed_value;
    }
    std::uint32_t magnitude{static_cast<std::uint32_t>(signed_value)};
    if (magnitude == 0U) {
        append_character(buffer, capacity, position, '0');
        return;
    }
    char digits[5]{};
    std::size_t digit_count{0U};
    while ((magnitude > 0U) && (digit_count < sizeof(digits))) {
        digits[digit_count] = static_cast<char>('0' + (magnitude % 10U));
        magnitude /= 10U;
        ++digit_count;
    }
    while (digit_count > 0U) {
        --digit_count;
        append_character(buffer, capacity, position, digits[digit_count]);
    }
}

[[nodiscard]] std::size_t format_sample(const application::sensors::AccelerationSample& sample,
                                        char* buffer,
                                        std::size_t capacity) noexcept {
    std::size_t position{0U};
    append_text(buffer, capacity, position, "X: ");
    append_integer(buffer, capacity, position, sample.x);
    append_text(buffer, capacity, position, ", Y: ");
    append_integer(buffer, capacity, position, sample.y);
    append_text(buffer, capacity, position, ", Z: ");
    append_integer(buffer, capacity, position, sample.z);
    return position;
}

} // namespace application::sensors