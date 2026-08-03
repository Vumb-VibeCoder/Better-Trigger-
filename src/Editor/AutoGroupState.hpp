#pragma once
// ============================================================================
// Trạng thái bật/tắt cho tính năng "Auto Group ID": khi bật, MỌI object được
// đặt vào editor sẽ tự động được gán 1 group ID còn trống (không trùng với
// group nào đang tồn tại trong level), lấy từ findFreeGroupId().
// ============================================================================

namespace ta {

// Trả về reference tới trạng thái bật/tắt hiện tại, dùng chung giữa
// AssistantPopup (nơi có cái tick) và EditorHooks (nơi hook createObject).
bool& autoGroupEnabled();

}