#ifndef APPL_SENSORS_INC_SENSOR_HPP
#define APPL_SENSORS_INC_SENSOR_HPP

#include <cstdint>

namespace application::sensors {

/**
 * @brief Describes the result of a sensor operation.
 */
enum class SensorStatus : std::uint8_t {
    ok,
    not_initialized,
    communication_error,
    unexpected_device,
    configuration_error
};

/**
 * @brief Generic interface for a sensor that produces samples.
 *
 * @tparam SampleType Type of sample produced by the sensor.
 */
template <typename SampleType>
class Sensor {
  public:
    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Initializes and configures the sensor.
     *
     * @return Status of the initialization operation.
     */
    [[nodiscard]] virtual SensorStatus initialize() noexcept = 0;

    /**
     * @brief Reads one sample from the sensor.
     *
     * @param sample Destination for the sensor sample.
     * @return Status of the read operation.
     */
    [[nodiscard]] virtual SensorStatus read(SampleType& sample) noexcept = 0;

    /**
     * @brief Checks whether the sensor has been initialized successfully.
     *
     * @return `true` when the sensor is initialized; otherwise `false`.
     */
    [[nodiscard]] virtual bool initialized() const noexcept = 0;

  protected:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Constructs the sensor interface.
     */
    Sensor() = default;

    /**
     * @brief Destroys the sensor interface.
     */
    ~Sensor() = default;

    Sensor(const Sensor&) = delete;
    Sensor& operator=(const Sensor&) = delete;
};

} // namespace application::sensors

#endif // APPL_SENSORS_INC_SENSOR_HPP