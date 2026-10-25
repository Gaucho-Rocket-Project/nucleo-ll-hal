#ifndef PLATFORM_INC_DIGITAL_OUTPUT_HPP
#define PLATFORM_INC_DIGITAL_OUTPUT_HPP

namespace platform {

/**
 * @brief Interface for controlling a digital output.
 */
class DigitalOutput {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Destroys the digital-output interface.
     */
    virtual ~DigitalOutput() = default;

    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Sets the output to its active state.
     */
    virtual void set() noexcept = 0;

    /**
     * @brief Sets the output to its inactive state.
     */
    virtual void clear() noexcept = 0;

    /**
     * @brief Toggles the output between its active and inactive states.
     */
    virtual void toggle() noexcept = 0;

    /**
     * @brief Writes the requested logical state to the output.
     *
     * @param active `true` to activate the output; `false` to deactivate it.
     */
    virtual void write(bool active) noexcept = 0;
}; // class DigitalOutput

} // namespace platform

#endif // PLATFORM_INC_DIGITAL_OUTPUT_H