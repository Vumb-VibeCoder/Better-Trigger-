// ============================================================================
// Adds four things to the editor's existing button toolbar
// ("editor-buttons-menu"):
//   1. Trigger Explainer button (unchanged).
//   2/3/4. Three checkboxes controlling PlacementAssist:
//        - "Auto Group ID": auto-assigns a fresh, non-colliding group id
//          (obj->addToGroup) to EVERY object you place, not just triggers.
//        - "Auto Tag ID": auto-assigns the next free Target Group id to
//          every trigger you place, per known trigger type.
//        - "Copy First": while the above is on, every trigger placed after
//          the first one of its type copies that first one's motion params.
//
// Also adds keyboard shortcuts for the right-side action buttons in the
// editor (Copy, Paste, Copy+Paste, Edit Special, Edit Group, Edit Object,
// Copy Values, Paste State, Paste Color, Go to Layer, De-Select), plus a
// hotkey (5) to open the Gemini AI popup.
//
// TOGGLE BEHAVIOR: for keys that open a popup (G, O, H, I), pressing the
// SAME key a second time closes that popup instead of opening a new one, by
// simulating an Escape keypress (which every GD/Geode popup responds to).
// This is tracked with m_lastPopupKey - if you press a DIFFERENT popup key
// while one is open, it just opens the new popup on top (GD's own default
// behavior), and m_lastPopupKey switches to the new one.
//
// The Gemini panel (key 5) is NOT a modal popup - it's a floating CCLayer,
// so it's toggled directly via setVisible() instead of the Escape-dispatch
// trick used for GD's own popups.
//
// LIMITATION: if a popup is closed some other way (clicking OK/X, clicking
// outside, etc.) m_lastPopupKey is not reset, since we can't hook into
// GD's own popups being closed. Pressing that key again in that case will
// simply reopen the popup as normal (the stray Escape dispatch is harmless
// if nothing is open to catch it).
//
// All hotkeys below use keys NOT used by vanilla GD editor shortcuts
// (which use W A S D Q E C V Ctrl+D 1 2 3, arrow keys, Alt+Q, Alt+E).
//
// IMPORTANT: The mapping from `editObject` / `editObjectSpecial` to the
// actual on-screen buttons ("Edit Object" vs "Edit Special") is a best
// guess based on function naming in the binding header -- NOT confirmed.
// Build this, test each key in the editor, and swap the calls around below
// if a key opens the wrong popup.
//
// NOTE: Group Remap feature has been removed (GroupRemap.hpp never
// existed). If you want it back later, re-add GroupRemap.hpp/.cpp and
// restore the remap button/toggle/hotkey blocks below.
// ============================================================================

#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/loader/Log.hpp>
#include "../UI/ExplainPopup.hpp"
#include "../Editor/FloatingGeminiPanel.hpp"
#include "PlacementAssist.hpp"

using namespace geode::prelude;

class $modify(TAEditorUI, EditorUI) {
    struct Fields {
        cocos2d::enumKeyCodes m_lastPopupKey = cocos2d::enumKeyCodes(0);
        ta::FloatingGeminiPanel* m_geminiPanel = nullptr;
    };

    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;

        auto menu = this->getChildByID("editor-buttons-menu");
        if (!menu) return true; // defensive: don't crash the editor if GD's layout changed

        if (auto spr = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png")) {
            spr->setScale(0.9f);
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(TAEditorUI::onOpenExplainer)
            );
            btn->setID("trigger-explainer-button"_spr);
            menu->addChild(btn);
        }

        auto groupToggle = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(TAEditorUI::onToggleAutoGroup), 0.7f
        );
        groupToggle->setID("trigger-auto-group-toggle"_spr);
        groupToggle->toggle(ta::g_autoGroupEnabled);
        menu->addChild(groupToggle);

        auto tagToggle = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(TAEditorUI::onToggleAutoTag), 0.7f
        );
        tagToggle->setID("trigger-auto-tag-toggle"_spr);
        tagToggle->toggle(ta::g_autoTagEnabled);
        menu->addChild(tagToggle);

        auto copyToggle = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(TAEditorUI::onToggleCopyFirst), 0.7f
        );
        copyToggle->setID("trigger-copy-first-toggle"_spr);
        copyToggle->toggle(ta::g_copyFirstEnabled);
        menu->addChild(copyToggle);

        menu->updateLayout();
        return true;
    }

    // Post-process every object the editor places (click, drag, paste...):
    // PlacementAssist applies whichever of the 3 toggles are on (auto
    // group, auto tag, copy-first).
    GameObject* createObject(int objID, cocos2d::CCPoint pos) {
        auto obj = EditorUI::createObject(objID, pos);
        if (ta::onObjectPlaced(obj, this->m_editorLayer)) {
            // Force the same visual refresh that normally only happens when
            // you click into the object by hand (group id label, etc.)
            this->selectObject(obj, false);
        }
        return obj;
    }

    void onOpenExplainer(CCObject*) {
        GameObject* selected = nullptr;
        if (this->m_selectedObject) {
            selected = this->m_selectedObject;
        } else if (this->m_selectedObjects && this->m_selectedObjects->count() == 1) {
            selected = static_cast<GameObject*>(this->m_selectedObjects->objectAtIndex(0));
        }
        ta::ExplainPopup::create(selected, this, this->m_editorLayer)->show();
    }

    void onToggleAutoGroup(CCObject*) {
        ta::g_autoGroupEnabled = !ta::g_autoGroupEnabled;
        ta::resetTemplates();
    }

    void onToggleAutoTag(CCObject*) {
        ta::g_autoTagEnabled = !ta::g_autoTagEnabled;
        ta::resetTemplates();
    }

    void onToggleCopyFirst(CCObject*) {
        ta::g_copyFirstEnabled = !ta::g_copyFirstEnabled;
        ta::resetTemplates();
    }

    // Creates the floating Gemini panel the first time it's opened, then
    // just shows/hides it on subsequent toggles (it's a CCLayer child of
    // this EditorUI, not a modal popup, so it has no show()/hide() pair --
    // we drive its visibility directly).
    void onOpenGeminiPanel() {
        if (!m_fields->m_geminiPanel) {
            m_fields->m_geminiPanel = ta::FloatingGeminiPanel::create(this, this->m_editorLayer);
            if (m_fields->m_geminiPanel) {
                this->addChild(m_fields->m_geminiPanel, 999);
            }
            return;
        }
        m_fields->m_geminiPanel->setVisible(!m_fields->m_geminiPanel->isVisible());
    }

    // Simulates pressing Escape, which every GD/Geode popup (FLAlertLayer,
    // Popup subclasses) listens for to close itself. Used to fake a
    // "press hotkey again to close" toggle for popups we don't own.
    //
    // Real signature: bool dispatchKeyboardMSG(enumKeyCodes key,
    //                                           bool isKeyDown,
    //                                           bool isKeyRepeat,
    //                                           double);
    void closeTopPopup() {
        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(
            cocos2d::KEY_Escape, true, false, 0.0
        );
    }

    // ------------------------------------------------------------------
    // Keyboard shortcuts for the right-side action buttons.
    // ------------------------------------------------------------------
    void keyDown(cocos2d::enumKeyCodes key, double timestamp) {
        switch (key) {
            case cocos2d::KEY_G: // Edit Group (Group ID) - press again to close
                if (m_fields->m_lastPopupKey == cocos2d::KEY_G) {
                    this->closeTopPopup();
                    m_fields->m_lastPopupKey = cocos2d::enumKeyCodes(0);
                } else {
                    this->editGroup(nullptr);
                    m_fields->m_lastPopupKey = cocos2d::KEY_G;
                }
                return;

            case cocos2d::KEY_O: // Edit Object - press again to close
                if (m_fields->m_lastPopupKey == cocos2d::KEY_O) {
                    this->closeTopPopup();
                    m_fields->m_lastPopupKey = cocos2d::enumKeyCodes(0);
                } else {
                    this->editObject(nullptr);
                    m_fields->m_lastPopupKey = cocos2d::KEY_O;
                }
                return;

            case cocos2d::KEY_H: // Edit Special (UNCONFIRMED mapping) - press again to close
                if (m_fields->m_lastPopupKey == cocos2d::KEY_H) {
                    this->closeTopPopup();
                    m_fields->m_lastPopupKey = cocos2d::enumKeyCodes(0);
                } else {
                    this->editObjectSpecial(-1);
                    m_fields->m_lastPopupKey = cocos2d::KEY_H;
                }
                return;

            case cocos2d::KEY_J: // Copy
                this->onCopy(nullptr);
                return;

            case cocos2d::KEY_K: // Paste
                this->onPaste(nullptr);
                return;

            case cocos2d::KEY_L: // Copy + Paste (emulated: copy then paste)
                this->onCopy(nullptr);
                this->onPaste(nullptr);
                return;

            case cocos2d::KEY_N: // Copy Values / Copy State
                this->onCopyState(nullptr);
                return;

            case cocos2d::KEY_M: // Paste State
                this->onPasteState(nullptr);
                return;

            case cocos2d::KEY_P: // Paste Color
                this->onPasteColor(nullptr);
                return;

            case cocos2d::KEY_I: // Go to Layer - press again to close
                if (m_fields->m_lastPopupKey == cocos2d::KEY_I) {
                    this->closeTopPopup();
                    m_fields->m_lastPopupKey = cocos2d::enumKeyCodes(0);
                } else {
                    this->onGoToLayer(nullptr);
                    m_fields->m_lastPopupKey = cocos2d::KEY_I;
                }
                return;

            case cocos2d::KEY_U: // De-Select
                this->onDeselectAll(nullptr);
                return;

            case cocos2d::KEY_Five: // Gemini AI panel - toggles show/hide
                this->onOpenGeminiPanel();
                return;

            default:
                break;
        }

        // Not one of ours -> let GD handle it normally.
        EditorUI::keyDown(key, timestamp);
    }
};