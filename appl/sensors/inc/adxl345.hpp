#ifndef APPL_SENSORS_INC_ADXL345_HPP
#define APPL_SENSORS_INC_ADXL345_HPP

#include "gpio.hpp"
#include "sensor.hpp"
#include "spi_device.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace application::sensors {

/**
 * @brief Raw three-axis acceleration sample from the ADXL345.
 */
struct AccelerationSample {
    std::int16_t x;
    std::int16_t y;
    std::int16_t z;
};

/**
 * @brief Application-layer driver for an ADXL345 accelerometer.
 *
 * Communicates with the sensor over SPI and owns the active-low GPIO output
 * used as the device chip-select signal.
 */
class Adxl345 final : public Sensor<AccelerationSample> {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Constructs an ADXL345 driver.
     *
     * @param spi SPI bus connected to the sensor.
     * @param chip_select_pin GPIO pin connected to the sensor's active-low
     * chip-select input.
     */
    Adxl345(platform::Spi& spi, platform::GpioPin chip_select_pin) noexcept;

    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Verifies and configures the ADXL345.
     *
     * Reads the device identifier, configures the output data rate and data
     * format, and places the sensor into measurement mode.
     *
     * @return Status of the initialization operation.
     */
    [[nodiscard]] SensorStatus initialize() noexcept override;

    /**
     * @brief Reads one three-axis acceleration sample.
     *
     * @param sample Destination for the raw acceleration sample.
     * @return Status of the read operation.
     */
    [[nodiscard]] SensorStatus read(AccelerationSample& sample) noexcept override;

    /**
     * @brief Checks whether the sensor initialized successfully.
     *
     * @return `true` when initialization completed successfully; otherwise
     * `false`.
     */
    [[nodiscard]] bool initialized() const noexcept override;

  private:
    /**
     * @brief ADXL345 register addresses used by the driver.
     */
    enum class Register : std::uint8_t {
        device_id = 0x00U,
        bandwidth_rate = 0x2CU,
        power_control = 0x2DU,
        data_format = 0x31U,
        data_x0 = 0x32U
    };

    // -------------------------------------------------------------------------
    // Private Member Methods

    /**
     * @brief Reads one ADXL345 register.
     *
     * @param register_address Register to read.
     * @param value Destination for the register value.
     * @return Status of the register read.
     */
    [[nodiscard]] SensorStatus read_register(Register register_address, std::uint8_t& value) noexcept;

    /**
     * @brief Reads consecutive ADXL345 registers.
     *
     * @param start_address First register to read.
     * @param values Destination buffer for the register values.
     * @param count Number of registers to read.
     * @return Status of the register read.
     */
    [[nodiscard]] SensorStatus read_registers(Register start_address, std::uint8_t* values, std::size_t count) noexcept;

    /**
     * @brief Writes one ADXL345 register.
     *
     * @param register_address Register to write.
     * @param value Value to write.
     * @return Status of the register write.
     */
    [[nodiscard]] SensorStatus write_register(Register register_address, std::uint8_t value) noexcept;

    /**
     * @brief Converts an SPI status into a sensor status.
     *
     * @param status SPI operation status.
     * @return Corresponding sensor operation status.
     */
    [[nodiscard]] static SensorStatus convert_status(platform::SpiStatus status) noexcept;

    /**
     * @brief Combines two register bytes into a signed acceleration value.
     *
     * @param low Least-significant byte.
     * @param high Most-significant byte.
     * @return Signed 16-bit acceleration value.
     */
    [[nodiscard]] static std::int16_t combine_bytes(std::uint8_t low, std::uint8_t high) noexcept;

    platform::SpiDevice device_;
    bool initialized_{false};

    static constexpr std::uint8_t expected_device_id{0xE5U};
    static constexpr std::uint8_t read_command{0x80U};
    static constexpr std::uint8_t multibyte_command{0x40U};
    static constexpr std::uint8_t standby_mode{0x00U};
    static constexpr std::uint8_t measurement_mode{0x08U};
    static constexpr std::uint8_t data_format_configuration{0x08U};
    static constexpr std::uint8_t bandwidth_configuration{0x0AU};
};

/**
 * @brief Converts a sensor status into printable text.
 *
 * @param status Sensor status to convert.
 * @return Text describing the sensor status.
 */
[[nodiscard]] constexpr std::string_view to_string(SensorStatus status) noexcept {
    switch (status) {
    case SensorStatus::ok:
        return "ok";

    case SensorStatus::not_initialized:
        return "not initialized";

    case SensorStatus::communication_error:
        return "communication error";

    case SensorStatus::unexpected_device:
        return "unexpected device";

    case SensorStatus::configuration_error:
        return "configuration error";
    }

    return "unknown sensor status";
}

/**
 * @brief Formats an acceleration sample as printable text.
 *
 * The resulting text has the following format:
 *
 * @code
 * X: 12, Y: -35, Z: 258
 * @endcode
 *
 * The function does not append a null terminator. The returned length should
 * be used when constructing a string view over the buffer.
 *
 * @param sample Acceleration sample to format.
 * @param buffer Destination buffer.
 * @param capacity Capacity of the destination buffer.
 * @return Number of characters written to the buffer.
 */
[[nodiscard]] std::size_t format_sample(const AccelerationSample& sample, char* buffer, std::size_t capacity) noexcept;

} // namespace application::sensors

#endif // APPL_SENSORS_INC_ADXL345_HPP