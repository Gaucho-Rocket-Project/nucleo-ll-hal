#ifndef PLATFORM_INC_GPIO_HPP
#define PLATFORM_INC_GPIO_HPP

#include "digital_input.hpp"
#include "digital_output.hpp"
#include "stm32l4xx.h"
#include <cstdint>

/**
 * @brief Defines which electrical level represents the active state.
 */
enum class ActiveLevel : std::uint8_t {
    low = false,
    high = true
};

namespace platform {

/**
 * @brief Identifies a physical GPIO pin.
 */
struct GpioPin {
    GPIO_TypeDef* port;
    std::uint32_t pin;
};

/**
 * @brief STM32 GPIO implementation of a digital output.
 */
class GpioOutput final : public DigitalOutput {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Constructs a GPIO output.
     *
     * @param port GPIO peripheral containing the output pin.
     * @param pin STM32 LL pin mask.
     * @param active_level Electrical level representing the active state.
     */
    explicit constexpr GpioOutput(GPIO_TypeDef* port,
                                  std::uint32_t pin,
                                  ::ActiveLevel active_level = ::ActiveLevel::high) noexcept
        : port_{port}, pin_{pin}, active_level_{active_level} {}

    /**
     * @brief Constructs a GPIO output from a pin descriptor.
     *
     * @param pin GPIO pin descriptor.
     * @param active_level Electrical level representing the active state.
     */
    explicit constexpr GpioOutput(GpioPin pin,
                                  ::ActiveLevel active_level = ::ActiveLevel::high) noexcept
        : GpioOutput{pin.port,
                     pin.pin,
                     active_level} {}

    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Sets the GPIO output to its active state.
     */
    void set() noexcept override;

    /**
     * @brief Sets the GPIO output to its inactive state.
     */
    void clear() noexcept override;

    /**
     * @brief Toggles the GPIO output between its active and inactive states.
     */
    void toggle() noexcept override;

    /**
     * @brief Writes the requested logical state to the GPIO output.
     *
     * @param active `true` to activate the output; `false` to deactivate it.
     */
    void write(bool active) noexcept override;

  private:
    GPIO_TypeDef* port_;
    std::uint32_t pin_;
    ActiveLevel active_level_;
}; // class GpioOutput

/**
 * @brief STM32 GPIO implementation of a digital input.
 */
class GpioInput final : public DigitalInput {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Constructs a GPIO input.
     *
     * @param port GPIO peripheral containing the input pin.
     * @param pin STM32 LL pin mask.
     * @param active_level Electrical level representing the active state.
     */
    constexpr GpioInput(GPIO_TypeDef* port,
                        std::uint32_t pin,
                        ::ActiveLevel active_level = ::ActiveLevel::high) noexcept
        : port_{port}, pin_{pin}, active_level_{active_level} {}

    /**
     * @brief Constructs a GPIO input from a pin descriptor.
     *
     * @param pin GPIO pin descriptor.
     * @param active_level Electrical level representing the active state.
     */
    constexpr GpioInput(GpioPin pin,
                        ::ActiveLevel active_level = ::ActiveLevel::high) noexcept
        : GpioInput{pin.port,
                    pin.pin,
                    active_level} {}

    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Reads the logical state of the GPIO input.
     *
     * @return `true` when the input is active; otherwise `false`.
     */
    [[nodiscard]] bool read() const noexcept override;

  private:
    GPIO_TypeDef* port_;
    std::uint32_t pin_;
    ActiveLevel active_level_;
}; // class GpioInput

} // namespace platform

#endif // PLATFORM_INC_GPIO_HPP