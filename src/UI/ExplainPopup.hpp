// ExplainPopup.hpp
#pragma once

#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

class GameObject;
class EditorUI;
class LevelEditorLayer;

namespace ta {

class ExplainPopup : public geode::Popup {
protected:
    geode::TextInput* m_queryInput = nullptr;

    // Ket qua "thuong" (mo ta trigger, thong bao loi...) - 1 label co
    // dinh, dat truc tiep vao m_mainLayer (KHONG dung ScrollLayer, vi
    // ScrollLayer tung gay loi hien thi lech vi tri ra ca goc man hinh).
    cocos2d::CCLabelBMFont* m_resultLabel = nullptr;

    // Container rieng cho cau tra loi co highlight mau (tu Gemini), cung
    // dat truc tiep vao m_mainLayer, thay the m_resultLabel khi hien.
    cocos2d::CCNode* m_resultRich = nullptr;

    // Shared flag so the background HTTP thread can tell whether this
    // popup instance is still alive before touching its members.
    std::shared_ptr<std::atomic<bool>> m_alive;

    // Context dung khi goi AI generate trigger (nut "Generate").
    EditorUI* m_editorUI = nullptr;
    LevelEditorLayer* m_editorLayer = nullptr;
    GameObject* m_selectedObj = nullptr;

    // Khong phai override — Popup khong co virtual init(GameObject*, EditorUI*, LevelEditorLayer*).
    // Day la overload rieng cua ExplainPopup, goi Popup::init(w, h) ben trong.
    bool init(GameObject* selected, EditorUI* ui, LevelEditorLayer* lel);

    void onQueryChanged(std::string const& text);
    void onAskAI(cocos2d::CCObject*);
    void onGenerateAI(cocos2d::CCObject*);

    static std::string describeSelected(GameObject* obj);

    // Hien thi text thuong (khong parse tag mau) - dung cho thong bao loi,
    // huong dan, mo ta trigger co san...
    void showPlainText(std::string const& text);

    // Hien thi text co the chua tag <r></r> (do), <g></g> (xanh la),
    // <p></p> (tim) - parse va dung nhieu CCLabelBMFont ghep lai, tu dong
    // xuong dong. Dung cho cau tra loi tu Gemini (nut Ask AI).
    void showRichText(std::string const& text);

    void clearResultRich();

public:
    // selected: object dang chon trong editor luc mo popup (co the null).
    // ui / lel: dung de spawn trigger moi khi bam nut "Generate".
    static ExplainPopup* create(GameObject* selected = nullptr, EditorUI* ui = nullptr, LevelEditorLayer* lel = nullptr);

    virtual ~ExplainPopup();
};

} // namespace ta