// FloatingGeminiPanel.hpp
#pragma once
// ============================================================================
// A small, draggable, NON-modal panel (unlike ExplainPopup, which is a
// full-screen-blocking FLAlertLayer/Popup). This is a plain CCLayer added
// directly as a child of EditorUI, so:
//   - it stays on screen while you keep editing/clicking/dragging objects
//     underneath it normally
//   - typing a command and hitting Generate spawns the trigger(s) via
//     Gemini + spawnObjectsFromAIJson, then immediately selects them so you
//     can drag/edit them right away without closing anything
//   - it can be dragged around by its title bar
// No opaque background - just floats over the editor.
//
// LAYOUT: title bar -> prompt input (+ history prev/next buttons) ->
// Generate/Clear buttons (ngay duoi prompt) -> khung log/ket qua (chiem
// phan con lai ben duoi).
//
// RESULT/LOG AREA: khong dung geode::ScrollLayer / cocos2d ScrollView (tung
// gay loi vi tri/clip trong ExplainPopup truoc day - xem comment o do). Thay
// vao do tu dung CCClippingNode lam khung xem CO DINH kich thuoc + keo tay
// thu cong de cuon noi dung ben trong. Font trong khu vuc nay LUON giu
// nguyen kLogTextScale, KHONG tu dong thu nho theo do dai text nhu
// ExplainPopup - noi dung dai se cuon duoc thay vi bi bop chu nho dan.
//
// PROMPT HISTORY: luu lai cac prompt da Generate thanh cong gan day nhat
// (giong lich su lenh terminal). Nut "<"/">" canh o input cho phep lui/tien
// giua cac prompt cu. Go tay lai (khong qua nut) se reset ve "ngoai lich su".
// ============================================================================

#include <Geode/ui/TextInput.hpp>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

class GameObject;
class EditorUI;
class LevelEditorLayer;

namespace ta {

class FloatingGeminiPanel : public cocos2d::CCLayer {
protected:
    cocos2d::CCLayerColor* m_titleBar = nullptr;
    geode::TextInput* m_input = nullptr;

    // ---- Khung log/ket qua: kich thuoc co dinh, cuon thu cong bang keo,
    // font khong bao gio tu thu nho. ----
    cocos2d::CCNode* m_logContent = nullptr;   // chua cac dong text da wrap
    std::vector<std::string> m_logLines;       // cac "entry" tho, chua wrap
    float m_logContentHeight = 0.f;            // tong chieu cao noi dung hien tai
    float m_logScrollY = 0.f;                  // offset cuon hien tai (0 = dau)
    cocos2d::CCPoint m_logAreaOrigin;          // vi tri bottom-left cua khung log (local)
    cocos2d::CCSize m_logAreaSize;

    // ---- Lich su prompt: danh sach cac lenh da Generate, moi nhat o cuoi.
    // m_historyIndex == (int)m_promptHistory.size() nghia la "ngoai lich su"
    // (dang go moi/tu do), khong bi dieu khien boi nut prev/next. ----
    std::vector<std::string> m_promptHistory;
    int m_historyIndex = 0;
    std::string m_draftBeforeHistory; // luu tam prompt dang go do khi bat dau luot history

    EditorUI* m_editorUI = nullptr;
    LevelEditorLayer* m_editorLayer = nullptr;

    std::shared_ptr<std::atomic<bool>> m_alive;

    bool m_dragging = false;
    cocos2d::CCPoint m_dragTouchStart;
    cocos2d::CCPoint m_dragPanelStart;

    // Keo cuon trong khung log - khac voi keo di chuyen ca panel qua title
    // bar, can flag rieng de phan biet 2 kieu keo cung mot touch delegate.
    bool m_logDragging = false;
    float m_logDragTouchStartY = 0.f;
    float m_logDragScrollStart = 0.f;

    static constexpr float kWidth = 260.f;
    static constexpr float kHeight = 220.f;
    static constexpr float kTitleBarHeight = 22.f;
    static constexpr float kLogTextScale = 0.35f; // CO DINH - khong tu thu nho

    // Khoang cach doc danh cho: input prompt + hang nut Generate/Clear,
    // tinh tu duoi title bar xuong. Phan con lai ben duoi la khung log.
    static constexpr float kInputRowHeight = 26.f;
    static constexpr float kButtonRowHeight = 34.f;

    // So luong prompt toi da luu trong lich su.
    static constexpr size_t kMaxHistory = 20;

    bool init(EditorUI* ui, LevelEditorLayer* lel);

    void onEnter() override;
    void onExit() override;

    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

    void onGenerate(cocos2d::CCObject*);
    void onClose(cocos2d::CCObject*);
    void onClear(cocos2d::CCObject*);

    // Nut lui/tien trong lich su prompt.
    void onHistoryPrev(cocos2d::CCObject*);
    void onHistoryNext(cocos2d::CCObject*);

    // Them 1 prompt vao lich su (bo qua neu trung voi entry cuoi cung).
    void pushPromptHistory(std::string const& query);

    // Them 1 "entry" text moi vao log (co the dai/nhieu dong), tu dong wrap
    // theo be rong khung voi kLogTextScale co dinh, roi tu cuon xuong cuoi
    // (dong moi nhat luon hien ra ngay, giong chat log).
    void appendLog(std::string const& text);
    void clearLog();
    void rebuildLogContent();
    void clampLogScroll();
    void applyLogScroll();

public:
    static FloatingGeminiPanel* create(EditorUI* ui, LevelEditorLayer* lel);
    virtual ~FloatingGeminiPanel();
};

} // namespace ta