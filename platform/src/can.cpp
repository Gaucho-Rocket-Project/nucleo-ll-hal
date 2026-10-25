#include "can.hpp"

namespace {

platform::CanStatus map_hal_status(HAL_StatusTypeDef status) noexcept {
    switch (status) {
        case HAL_OK:      return platform::CanStatus::ok;
        case HAL_BUSY:    return platform::CanStatus::busy;
        case HAL_TIMEOUT: return platform::CanStatus::timeout;
        default:          return platform::CanStatus::hal_error;
    }
}

} // namespace

namespace platform {

CanStatus Can::initialize() noexcept {
    if (started_) return CanStatus::ok;
    if (HAL_CAN_GetState(handle_) != HAL_CAN_STATE_READY) return CanStatus::not_ready;
    
    // 32-bit mask filter accepting all IDs into FIFO0 (loopback smoke test).
    CAN_FilterTypeDef filter{};
    filter.FilterBank = 0U;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = 0U;
    filter.FilterIdLow = 0U;
    filter.FilterMaskIdHigh = 0U;
    filter.FilterMaskIdLow = 0U;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14U;  // Used on dual-bxCAN devices; harmless on CAN1-only L476.

    auto result = HAL_CAN_ConfigFilter(handle_, &filter);
    if (result != HAL_OK) return map_hal_status(result);
    
    result = HAL_CAN_Start(handle_);
    if (result != HAL_OK) return map_hal_status(result);

    started_ = true;
    return CanStatus::ok;
}

CanStatus Can::send(const CanFrame& frame) noexcept {
    if (!started_) return CanStatus::not_ready;
    if (frame.id > 0x7FFU || frame.length > 8U) return CanStatus::invalid_frame;
    if (is_bus_off()) return CanStatus::bus_off;
    if (HAL_CAN_GetTxMailboxesFreeLevel(handle_) == 0U) return CanStatus::busy;
    
    // Construct the HAL transmit header for a standard data frame.
    CAN_TxHeaderTypeDef header{};
    header.StdId = frame.id;
    header.ExtId = 0U;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = frame.length;
    header.TransmitGlobalTime = DISABLE;

    auto payload = frame.data;
    std::uint32_t mailbox = 0U;
    const auto result = HAL_CAN_AddTxMessage(handle_, &header, payload.data(), &mailbox);
    if (result != HAL_OK && is_bus_off()) return CanStatus::bus_off;
    return map_hal_status(result);
}

CanStatus Can::receive(CanFrame& frame) noexcept {
    if (!started_) return CanStatus::not_ready;
    if (HAL_CAN_GetRxFifoFillLevel(handle_, CAN_RX_FIFO0) == 0U) return CanStatus::empty;

    CAN_RxHeaderTypeDef header{};
    std::array<std::uint8_t, 8> payload{};
    const auto result = HAL_CAN_GetRxMessage(handle_, CAN_RX_FIFO0, &header, payload.data());
    if (result != HAL_OK) return map_hal_status(result);

    // Standard data frames only.
    if (header.IDE != CAN_ID_STD || header.RTR != CAN_RTR_DATA || header.DLC > 8U) {
        return CanStatus::invalid_frame;
    }

    frame.id = header.StdId;
    frame.length = static_cast<std::uint8_t>(header.DLC);
    frame.data = payload;
    return CanStatus::ok;
}

bool Can::is_bus_off() const noexcept {
    // Current controller state rather than latched historical HAL errors.
    return (handle_->Instance->ESR & CAN_ESR_BOFF) != 0U;
}

std::uint32_t Can::error_code() const noexcept {
    return HAL_CAN_GetError(handle_);
}

} // namespace platform
