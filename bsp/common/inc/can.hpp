
#ifndef PLATFORM_INC_CAN_HPP
#define PLATFORM_INC_CAN_HPP

#include "mcu.hpp"
#include BSP_HAL_CAN_HEADER

#include <array>
#include <cstdint>

namespace bsp {

/**
 * @brief Represents a standard Classic CAN data frame.
 *
 * @note Extended identifiers, remote frames, and CAN FD are not supported
 * by this initial implementation.
 */
struct CanFrame {
    std::uint32_t id{0};                 // Standard 11-bit identifier (0x000-0x7FF).
    std::uint8_t length{0};              // Number of payload bytes (0-8).
    std::array<std::uint8_t, 8> data{};  // CAN payload; only the first length bytes are valid.
};

/**
 * @brief Describes the result of a CAN operation.
 */
enum class CanStatus : std::uint8_t {
    ok,
    empty,
    busy,
    invalid_frame,
    not_ready,
    bus_off,
    timeout,
    hal_error
};

/**
 * @brief Polling Classic CAN interface for an STM32 bxCAN peripheral.
 *
 * @details Uses an externally initialized HAL CAN handle. The driver
 * configures an accept-all FIFO0 filter and starts the peripheral, then
 * provides nonblocking transmit-queue and receive operations.
 *
 * @note Must execute the CubeMX-generated MX_CAN1_Init()
 * before calling initialize(). No CAN interrupts are used by this driver.
 */
class Can final {
  public:
    // -------------------------------------------------------------------------
    // Public Constructors and Destructors
    /**
     * @brief Constructs a CAN interface.
     *
     * @param handle Reference to the HAL CAN handle (for example, hcan1).
     * @note The referenced handle must remain valid for the lifetime of Can.
     */
    explicit Can(CAN_HandleTypeDef& handle) noexcept : handle_{&handle} {};

    // -------------------------------------------------------------------------
    // Public Member Methods
    /**
     * @brief Configures the CAN receive filter and starts the CAN peripheral.
     *
     * @details Configures a 32-bit mask filter that accepts all identifiers
     * into RX FIFO0.
     * @return CanStatus::ok on success; otherwise a status describing the failure.
     */
    [[nodiscard]] CanStatus initialize() noexcept;

    /**
     * @brief Queues a standard CAN data frame for transmission.
     *
     * @param frame Frame containing an 11-bit identifier, length, and data.
     * @return CanStatus::ok if HAL accepted the frame into a TX mailbox;
     *         CanStatus::busy if no mailbox is free; or another error status.
     */
    [[nodiscard]] CanStatus send(const CanFrame& frame) noexcept;

    /**
     * @brief Polls RX FIFO0 for a received standard CAN data frame.
     *
     * @param[out] frame Receives the identifier, length, and payload on success.
     * @return CanStatus::ok when a supported frame is read;
     *         CanStatus::empty when FIFO0 contains no frames; or an error status.
     * @note This operation does not block. Unsupported received frame types
     * are removed from FIFO0 and reported as CanStatus::invalid_frame.
     */
    [[nodiscard]] CanStatus receive(CanFrame& frame) noexcept;

    /**
     * @brief Checks whether the CAN controller is currently bus-off.
     *
     * @return true if the hardware bus-off flag is set; otherwise false.
     */
    [[nodiscard]] bool is_bus_off() const noexcept;

    /**
     * @brief Returns the error flags stored by the STM32 HAL CAN driver.
     *
     * @return HAL CAN error bitmask; interpret using HAL_CAN_ERROR_* flags.
     */
    [[nodiscard]] std::uint32_t error_code() const noexcept;

  private:
    CAN_HandleTypeDef* handle_;
    bool started_{false};
};

} // namespace bsp

#endif // PLATFORM_INC_CAN_HPP
