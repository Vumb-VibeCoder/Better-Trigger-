#pragma once
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/utils/general.hpp>

namespace ta {

// Toggle 1: auto-assign a fresh, non-colliding GROUP ID (obj->addToGroup) to
// EVERY object placed via click - not just triggers, not the target
// group/tag. Scans the whole level fresh each time so freed ids get reused.
extern bool g_autoGroupEnabled;

// Toggle 2: auto-assign the next free TARGET GROUP ID (the "tag" a trigger
// acts on, m_targetGroupID) to every known trigger object placed via click,
// instead of leaving it at 0. Only applies to objects recognized in
// TriggerSchema.
extern bool g_autoTagEnabled;

// Toggle 3: the FIRST trigger of a given type placed after this was turned
// on becomes the "live template" - every following trigger of that same
// type copies its CURRENT motion params (duration, easing, moveX/Y,
// degrees...) at the moment of placement, so edits you make to the first
// trigger afterward are picked up too. Only applies to known trigger types.
extern bool g_copyFirstEnabled;

// Khi true, onObjectPlaced() không làm gì cả (bỏ qua cả 3 toggle ở trên),
// dù chúng đang bật. Dùng để các nguồn tạo object KHÁC người dùng tự click
// (vd AI Generate) không bị dính tự động group/tag/copy-first ngoài ý
// muốn - AI tự set field của nó rồi.
extern bool g_suppressPlacementAssist;

// RAII guard cho g_suppressPlacementAssist: set true khi tao, tra ve gia
// tri cu khi huy (kha nang loi long guard, vd goi tu 2 cho khac nhau).
// Dung thay cho "g_suppressPlacementAssist = true; ... = false;" thu cong -
// neu co exception hoac return som giua vong lap (vd AIGenerate ném lỗi
// giữa lúc parse/tạo object), destructor vẫn được gọi và cờ không bị kẹt ở
// true mãi mãi (kẹt thì user sẽ mất luôn cả 3 toggle Auto Group/Auto Tag/
// Copy First mà không rõ vì sao).
class SuppressPlacementAssistGuard {
public:
    SuppressPlacementAssistGuard() : m_prev(g_suppressPlacementAssist) {
        g_suppressPlacementAssist = true;
    }
    ~SuppressPlacementAssistGuard() {
        g_suppressPlacementAssist = m_prev;
    }
    SuppressPlacementAssistGuard(SuppressPlacementAssistGuard const&) = delete;
    SuppressPlacementAssistGuard& operator=(SuppressPlacementAssistGuard const&) = delete;
private:
    bool m_prev;
};

// Called right after EditorUI::createObject places a new object. Applies
// whichever of the three toggles above are enabled. Returns true if
// anything was changed, so the caller can re-select the object to refresh
// its on-screen labels/params right away.
bool onObjectPlaced(GameObject* obj, LevelEditorLayer* lel);

// Clears the "first trigger" reference for every type. Call this whenever
// any toggle changes state, so behavior always restarts clean instead of
// mixing an old template with new intent.
void resetTemplates();

}