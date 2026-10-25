#include "adxl345.hpp"
#include "nucleo_l476rg.hpp"
#include "system_clock.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

/**
 * @brief Application entry point.
 * @return This function does not return.
 */
int main() {
    namespace board = platform::nucleo_l476rg;
    namespace sensors = application::sensors;

    board::initialize();

    auto& console = board::console();

    console.write_line("ADXL345 acceleration test");

    sensors::Adxl345 accelerometer{board::spi1(),
                                   board::pins::pa4};

    const sensors::SensorStatus initialization_status = accelerometer.initialize();

    if (initialization_status != sensors::SensorStatus::ok) {
        console.write_line("ADXL345 initialization failed:");
        console.write_line(to_string(initialization_status));

        while (true); // Stop here so that the initialization error remains visible.
    }

    console.write_line("ADXL345 initialized successfully");

    while (true) {
        sensors::AccelerationSample sample{};

        const sensors::SensorStatus read_status{
            accelerometer.read(sample)};

        if (read_status == sensors::SensorStatus::ok) {
            char message[64]{};

            const std::size_t message_length{format_sample(sample,
                                                           message,
                                                           sizeof(message))};

            console.write_line(std::string_view{message,
                                                message_length});
        } else {
            console.write_line("ADXL345 read failed:");
            console.write_line(to_string(read_status));
        }

        /*
         * Read and print approximately ten samples per second.
         */
        platform::clock::delay_ms(100U);
    }
}