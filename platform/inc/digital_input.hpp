#ifndef PLATFORM_INC_DIGITAL_INPUT_HPP
#define PLATFORM_INC_DIGITAL_INPUT_HPP

namespace platform {

/**
 * @brief Interface for reading a digital input.
 */
class DigitalInput {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors

    /**
     * @brief Destroys the digital-input interface.
     */
    virtual ~DigitalInput() = default;

    // -------------------------------------------------------------------------
    // Public Member Methods

    /**
     * @brief Reads the logical state of the input.
     *
     * @return `true` when the input is active; otherwise `false`.
     */
    [[nodiscard]] virtual bool read() const noexcept = 0;
}; // class DigitalInput

} // namespace platform

#endif