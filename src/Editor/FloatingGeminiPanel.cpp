// FloatingGeminiPanel.cpp
#include "FloatingGeminiPanel.hpp"
#include "AIGenerate.hpp"
#include "TrustedTriggerList.hpp"
#include "GeminiClient.hpp"
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <sstream>

using namespace geode::prelude;

namespace ta {

namespace {
    // Wrap 1 doan text (co the co nhieu dong \n san) thanh danh sach dong
    // hien thi, moi dong khong vuot qua maxWidth o scale cho truoc. Dung
    // cho khung log - KHONG lien quan gi den logic tu thu nho font (khung
    // log khong bao gio doi scale).
    std::vector<std::string> wrapPlainText(std::string const& text, float maxWidth, float scale) {
        std::vector<std::string> lines;
        std::istringstream paragraphs(text);
        std::string paragraph;

        while (std::getline(paragraphs, paragraph, '\n')) {
            if (paragraph.empty()) {
                lines.push_back("");
                continue;
            }

            std::istringstream words(paragraph);
            std::string word;
            std::string current;

            while (words >> word) {
                std::string candidate = current.empty() ? word : current + " " + word;
                auto tmp = cocos2d::CCLabelBMFont::create(candidate.c_str(), "chatFont.fnt");
                float w = tmp->getContentSize().width * scale;

                if (w > maxWidth && !current.empty()) {
                    lines.push_back(current);
                    current = word;
                } else {
                    current = candidate;
                }
            }
            if (!current.empty()) lines.push_back(current);
        }

        if (lines.empty()) lines.push_back("");
        return lines;
    }
}

FloatingGeminiPanel* FloatingGeminiPanel::create(EditorUI* ui, LevelEditorLayer* lel) {
    auto ret = new FloatingGeminiPanel();
    if (ret->init(ui, lel)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

FloatingGeminiPanel::~FloatingGeminiPanel() {
    if (m_alive) m_alive->store(false);
}

bool FloatingGeminiPanel::init(EditorUI* ui, LevelEditorLayer* lel) {
    if (!CCLayer::init()) return false;

    m_editorUI = ui;
    m_editorLayer = lel;
    m_alive = std::make_shared<std::atomic<bool>>(true);

    this->setContentSize(cocos2d::CCSize(kWidth, kHeight));
    this->setAnchorPoint(cocos2d::CCPoint(0.f, 0.f));

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    this->setPosition(cocos2d::CCPoint(150.f, winSize.height - kHeight - 60.f));

    // Chi thanh title co nen mau thoi (cho de nhin thay va keo tha duoc),
    // con lai de trong suot luon, thuoc troi troi tren editor khong che
    // gi ca - cho do vuong mat.
    m_titleBar = cocos2d::CCLayerColor::create({60, 40, 90, 220}, kWidth, kTitleBarHeight);
    m_titleBar->setPosition({0.f, kHeight - kTitleBarHeight});
    this->addChild(m_titleBar);

    auto titleLabel = cocos2d::CCLabelBMFont::create("May Nan Trigger (Gemini)", "chatFont.fnt");
    titleLabel->setScale(0.42f);
    titleLabel->setAnchorPoint({0.f, 0.5f});
    titleLabel->setPosition({8.f, kHeight - kTitleBarHeight / 2.f});
    this->addChild(titleLabel);

    auto closeMenu = cocos2d::CCMenu::create();
    closeMenu->setPosition({0.f, 0.f});
    if (auto closeSpr = cocos2d::CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png")) {
        closeSpr->setScale(0.5f);
        auto closeBtn = CCMenuItemSpriteExtra::create(closeSpr, this, menu_selector(FloatingGeminiPanel::onClose));
        closeBtn->setPosition({kWidth - 14.f, kHeight - kTitleBarHeight / 2.f});
        closeMenu->addChild(closeBtn);
    }
    this->addChild(closeMenu);

    // ---- Vung noi dung ben duoi title bar, tu tren xuong: prompt input
    // (+ 2 nut lich su prev/next 2 ben) -> hang nut Generate/Clear -> khung
    // log (chiem het phan con lai). ----
    float contentTop = kHeight - kTitleBarHeight;

    float inputY = contentTop - kInputRowHeight;

    // Thu hep input lai de chua 2 nut prev/next lich su o 2 ben.
    const float historyBtnWidth = 16.f;
    m_input = geode::TextInput::create(kWidth - 20.f - historyBtnWidth * 2.f - 8.f,
                                        "vd: keo cai nay bay len 100 don vi coi...");
    m_input->setPosition({kWidth / 2.f, inputY});

    // Cho phep go chu, so, dau cau va cac ky hieu thuong dung trong prompt
    // (,.=<>...) - mac dinh TextInput loc bot ky tu nen phai mo rong filter
    // ro rang, giong cach mod.json khai bao "filter" cho setting string.
    m_input->setFilter(
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789 "
        ".,=_-+*/()[]{}!?:;'\"<>%$#@&|^~`\n"
    );
    this->addChild(m_input);

    auto historyMenu = cocos2d::CCMenu::create();
    historyMenu->setPosition({0.f, 0.f});

    auto prevSpr = ButtonSprite::create("<", "goldFont.fnt", "GJ_button_04.png", .6f);
    prevSpr->setScale(0.6f);
    auto prevBtn = CCMenuItemSpriteExtra::create(prevSpr, this, menu_selector(FloatingGeminiPanel::onHistoryPrev));
    prevBtn->setPosition({historyBtnWidth / 2.f + 2.f, inputY});
    historyMenu->addChild(prevBtn);

    auto nextSpr = ButtonSprite::create(">", "goldFont.fnt", "GJ_button_04.png", .6f);
    nextSpr->setScale(0.6f);
    auto nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(FloatingGeminiPanel::onHistoryNext));
    nextBtn->setPosition({kWidth - historyBtnWidth / 2.f - 2.f, inputY});
    historyMenu->addChild(nextBtn);

    this->addChild(historyMenu);

    // Nut Generate/Clear dat NGAY DUOI prompt, khong con nam duoi cung panel.
    float buttonY = inputY - kButtonRowHeight;

    auto buttonMenu = cocos2d::CCMenu::create();
    buttonMenu->setPosition({0.f, 0.f});

    auto genSpr = ButtonSprite::create("Generate", "goldFont.fnt", "GJ_button_01.png", .7f);
    genSpr->setScale(0.7f);
    auto genBtn = CCMenuItemSpriteExtra::create(genSpr, this, menu_selector(FloatingGeminiPanel::onGenerate));
    genBtn->setPosition({kWidth / 2.f - 40.f, buttonY});
    buttonMenu->addChild(genBtn);

    auto clearSpr = ButtonSprite::create("Clear", "goldFont.fnt", "GJ_button_06.png", .7f);
    clearSpr->setScale(0.7f);
    auto clearBtn = CCMenuItemSpriteExtra::create(clearSpr, this, menu_selector(FloatingGeminiPanel::onClear));
    clearBtn->setPosition({kWidth / 2.f + 40.f, buttonY});
    buttonMenu->addChild(clearBtn);

    this->addChild(buttonMenu);

    // ---- Khung log/ket qua: lap day phan con lai ben duoi hang nut. ----
    const float logMarginBottom = 10.f;
    const float logTopY = buttonY - kButtonRowHeight * 0.5f - 6.f;

    m_logAreaSize = cocos2d::CCSize(kWidth - 20.f, logTopY - logMarginBottom);
    m_logAreaOrigin = cocos2d::CCPoint(10.f, logMarginBottom);

    // Khong dung CCClippingNode - da xac nhan clip/mask khong hoat dong dung
    // trong ban cocos2d-x cua GD (giong ly do bo ScrollLayer truoc do). Thay
    // vao do dung 1 CCLayerColor lam nen + CCNode chua text, khong clip that
    // su - noi dung dai se TRAN RA NGOAI khung thay vi bi cat, nhung chac
    // chan hien thi duoc.
    auto logBg = cocos2d::CCLayerColor::create({0, 0, 0, 90}, m_logAreaSize.width, m_logAreaSize.height);
    logBg->setPosition(m_logAreaOrigin);
    this->addChild(logBg);

    m_logContent = cocos2d::CCNode::create();
    m_logContent->setAnchorPoint({0.f, 1.f});
    m_logContent->setPosition(m_logAreaOrigin + cocos2d::CCPoint(0.f, m_logAreaSize.height));
    this->addChild(m_logContent);

    appendLog("Ok san sang choi tiep.");

    return true;
}

void FloatingGeminiPanel::onEnter() {
    CCLayer::onEnter();
    cocos2d::CCTouchDispatcher::get()->addTargetedDelegate(this, -10000, true);
}

void FloatingGeminiPanel::onExit() {
    cocos2d::CCTouchDispatcher::get()->removeDelegate(this);
    CCLayer::onExit();
}

bool FloatingGeminiPanel::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent*) {
    // Neu panel dang an (setVisible(false) sau khi bam phim 5 lan 2), KHONG
    // duoc nuot touch - tra ve false ngay de touch roi xuong cac layer/nut
    // ben duoi (editor) nhu binh thuong. Truoc day thieu check nay khien
    // vung ma panel tung chiem giu van bi "khoa" du panel da an.
    if (!this->isVisible()) {
        return false;
    }

    auto local = this->convertTouchToNodeSpace(touch);

    bool inTitleBar = local.x >= 0.f && local.x <= kWidth &&
                       local.y >= (kHeight - kTitleBarHeight) && local.y <= kHeight;
    if (inTitleBar) {
        m_dragging = true;
        m_dragTouchStart = touch->getLocation();
        m_dragPanelStart = this->getPosition();
        return true;
    }

    bool inLogArea = local.x >= m_logAreaOrigin.x && local.x <= m_logAreaOrigin.x + m_logAreaSize.width &&
                      local.y >= m_logAreaOrigin.y && local.y <= m_logAreaOrigin.y + m_logAreaSize.height;
    if (inLogArea) {
        m_logDragging = true;
        m_logDragTouchStartY = touch->getLocation().y;
        m_logDragScrollStart = m_logScrollY;
        return true;
    }

    return false;
}

void FloatingGeminiPanel::ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent*) {
    if (m_dragging) {
        auto cur = touch->getLocation();
        auto delta = cur - m_dragTouchStart;
        this->setPosition(m_dragPanelStart + delta);
        return;
    }

    if (m_logDragging) {
        float curY = touch->getLocation().y;
        float delta = curY - m_logDragTouchStartY;
        // Keo xuong (delta duong) -> lui ve dau noi dung; keo len (delta am)
        // -> tien ve cuoi noi dung. Kieu cuon tu nhien thong thuong.
        m_logScrollY = m_logDragScrollStart - delta;
        clampLogScroll();
        applyLogScroll();
    }
}

void FloatingGeminiPanel::ccTouchEnded(cocos2d::CCTouch*, cocos2d::CCEvent*) {
    m_dragging = false;
    m_logDragging = false;
}

void FloatingGeminiPanel::ccTouchCancelled(cocos2d::CCTouch*, cocos2d::CCEvent*) {
    m_dragging = false;
    m_logDragging = false;
}

void FloatingGeminiPanel::onClose(cocos2d::CCObject*) {
    this->setVisible(false);
}

void FloatingGeminiPanel::onClear(cocos2d::CCObject*) {
    if (m_input) m_input->setString("");
    clearLog();
    appendLog("Ok san sang choi tiep.");
}

void FloatingGeminiPanel::pushPromptHistory(std::string const& query) {
    if (query.empty()) return;

    // Bo qua neu trung voi entry gan nhat, tranh lich su bi lap lai lien tuc
    // khi nguoi dung bam Generate nhieu lan cung 1 prompt.
    if (!m_promptHistory.empty() && m_promptHistory.back() == query) {
        m_historyIndex = (int)m_promptHistory.size();
        return;
    }

    m_promptHistory.push_back(query);
    if (m_promptHistory.size() > kMaxHistory) {
        m_promptHistory.erase(m_promptHistory.begin());
    }

    // Sau khi Generate, con tro luon dung "ngoai lich su" (san sang go moi).
    m_historyIndex = (int)m_promptHistory.size();
}

void FloatingGeminiPanel::onHistoryPrev(cocos2d::CCObject*) {
    if (m_promptHistory.empty()) return;

    // Lan dau bam lui tu trang thai "ngoai lich su" - luu tam noi dung dang
    // go de co the quay lai dung bang nut Next sau nay.
    if (m_historyIndex == (int)m_promptHistory.size()) {
        m_draftBeforeHistory = m_input ? m_input->getString() : "";
    }

    if (m_historyIndex > 0) {
        m_historyIndex--;
    }

    if (m_input && m_historyIndex >= 0 && m_historyIndex < (int)m_promptHistory.size()) {
        m_input->setString(m_promptHistory[m_historyIndex]);
    }
}

void FloatingGeminiPanel::onHistoryNext(cocos2d::CCObject*) {
    if (m_promptHistory.empty()) return;

    if (m_historyIndex < (int)m_promptHistory.size() - 1) {
        m_historyIndex++;
        if (m_input) m_input->setString(m_promptHistory[m_historyIndex]);
    } else if (m_historyIndex == (int)m_promptHistory.size() - 1) {
        // Di qua khoi entry cuoi cung - tra ve draft nguoi dung dang go do
        // truoc khi bat dau luot lich su.
        m_historyIndex = (int)m_promptHistory.size();
        if (m_input) m_input->setString(m_draftBeforeHistory);
    }
}

void FloatingGeminiPanel::rebuildLogContent() {
    if (!m_logContent) return;
    m_logContent->removeAllChildren();

    const float padding = 6.f;
    const float maxWidth = m_logAreaSize.width - padding * 2.f;
    const float lineHeight = kLogTextScale * 34.f;

    float y = 0.f;
    for (auto& raw : m_logLines) {
        auto wrapped = wrapPlainText(raw, maxWidth, kLogTextScale);
        for (auto& line : wrapped) {
            auto lbl = cocos2d::CCLabelBMFont::create(line.empty() ? " " : line.c_str(), "chatFont.fnt");
            lbl->setScale(kLogTextScale);
            lbl->setAnchorPoint({0.f, 1.f});
            lbl->setPosition({padding, -y});
            m_logContent->addChild(lbl);
            y += lineHeight;
        }
        y += lineHeight * 0.3f; // khoang cach nho giua cac entry
    }

    m_logContentHeight = y;
}

void FloatingGeminiPanel::clampLogScroll() {
    float maxScroll = m_logContentHeight - m_logAreaSize.height;
    if (maxScroll < 0.f) maxScroll = 0.f;
    if (m_logScrollY < 0.f) m_logScrollY = 0.f;
    if (m_logScrollY > maxScroll) m_logScrollY = maxScroll;
}

void FloatingGeminiPanel::applyLogScroll() {
    if (!m_logContent) return;
    m_logContent->setPositionY(m_logAreaSize.height + m_logScrollY);
}

void FloatingGeminiPanel::appendLog(std::string const& text) {
    m_logLines.push_back(text);

    // Gioi han so entry de tranh phinh to vo han khi dung lau.
    if (m_logLines.size() > 200) {
        m_logLines.erase(m_logLines.begin(), m_logLines.begin() + (m_logLines.size() - 200));
    }

    rebuildLogContent();

    // Tu dong cuon xuong dong moi nhat.
    m_logScrollY = 1e9f;
    clampLogScroll();
    applyLogScroll();
}

void FloatingGeminiPanel::clearLog() {
    m_logLines.clear();
    rebuildLogContent();
    m_logScrollY = 0.f;
    applyLogScroll();
}

void FloatingGeminiPanel::onGenerate(cocos2d::CCObject*) {
    auto query = m_input ? m_input->getString() : "";
    if (query.empty()) {
        appendLog("Go gi vao roi hay bam Generate chu, trong khong the doan y duoc.");
        return;
    }
    if (!m_editorUI || !m_editorLayer) {
        appendLog("Editor dau roi ta, chac ban thoat ra ngoai luc nao khong hay.");
        return;
    }

    // Luu prompt vao lich su NGAY khi bam Generate (khong doi ket qua AI
    // thanh cong hay khong) - vi nguoi dung van muon xem/tai su dung lai
    // prompt cu du lan do AI tra loi loi.
    pushPromptHistory(query);

    appendLog("> " + query);
    appendLog("Doi chut, dang nan Gemini lam viec...");

    GameObject* selected = m_editorUI->m_selectedObject;

    std::string context;
    if (selected) {
        context = "Object dang chon: ID " + std::to_string(selected->m_objectID);
        if (selected->m_groupCount > 0) {
            context += ", group " + std::to_string(selected->m_groups->at(0));
        }
        context += ". Vi tri (" + std::to_string(selected->getPositionX()) + ", "
                 + std::to_string(selected->getPositionY()) + ").\n";
    }

    std::string prompt = std::string(
        "Ban la tro ly tao trigger cho Geometry Dash 2.2. CHI tra loi bang JSON hop le, "
"KHONG them text giai thich, KHONG dung markdown code fence, KHONG dung dau ```. "
"Dinh dang chinh xac:\n\n"
"{\"objects\":[{\"id\":<object id so nguyen, BAT BUOC LAY TU DANH SACH DUOC XAC NHAN BEN "
"DUOI, KHONG duoc dung id nao khac du co ve hop ly>,\"x\":<offset X so voi object goc, mac "
"dinh 30>,\"y\":<offset Y, mac dinh 0>, ...cac tham so khac tuy theo loai trigger...}]}\n\n"

"NGUYEN TAC QUAN TRONG NHAT - KHONG DUOC BIA:\n"
"- Danh sach trigger va tham so ben duoi la NGUON DUY NHAT duoc phep dung. Moi trigger chi "
"duoc dung DUNG cac ten tham so da liet ke rieng cho no (kem kieu du lieu va khoang gia tri). "
"TUYET DOI KHONG tu dat ten tham so moi, KHONG doan ten tham so tu trigger khac, KHONG dung "
"tham so cua Move/Rotate/Scale cho trigger khong co no.\n"
"- Neu nguoi dung yeu cau mot hieu ung/tham so KHONG co trong danh sach tham so cua trigger "
"do, BO QUA yeu cau do (khong dien bay tham so), KHONG duoc gan gia tri vao mot tham so gan "
"dung ten de \"co gang dap ung\".\n"
"- Neu khong chac trigger nao phu hop yeu cau, chon trigger gan nghia nhat trong danh sach "
"duoc xac nhan - KHONG duoc bia id.\n\n"

"QUY DOI DON VI VI TRI (x/y cua object, offset dat object): don vi la UNIT THUC trong game, "
"1 o luoi = 30 unit. Neu nguoi dung noi khoang cach theo SO O/BLOCK (vd \"cach 100 block\", "
"\"cach 5 o\"), PHAI nhan so do voi 30 de ra offset x/y - vi du \"cach 100 block\" nghia la "
"x=3000, KHONG duoc dien thang so 100.\n\n"

"QUY DOI RIENG CHO moveX/moveY (Move Trigger, Area/Enter Move Trigger): day la DON VI HIEN "
"THI GIONG POPUP MOVE TRIGGER TRONG GAME (khong phai pixel, khong phai unit dat object o tren) "
"- ty le co dinh 10 don vi hien thi = 1 o luoi. Neu nguoi dung noi \"di chuyen X o/block\", "
"PHAI nhan X voi 10 de ra moveX/moveY (vd \"di chuyen 2 o sang phai\" => moveX=20). Neu nguoi "
"dung cho thang so \"don vi\"/\"unit\" trong popup Move Trigger thi dien nguyen so do, khong "
"nhan gi them.\n\n"

"KHI TAO NHIEU TRIGGER CUNG LUC: moi \"x\" trong mang objects deu la offset TINH TU OBJECT "
"GOC (khong phai tinh tu trigger truoc do), nen PHAI tu TANG DAN gia tri x cho tung trigger "
"de chung khong bi chong len nhau. Vi du can 3 trigger cach nhau 100 unit, phai tra ve x lan "
"luot la 100, 200, 300 (KHONG duoc de ca 3 cung x=30 hay x=100).\n\n"

"DANH SACH CHANCE (\"chanceList\" - dung cho random_trigger/advanced_random_trigger/"
"sequence_trigger): la mot MANG cac object {\"groupId\":<int>,\"chance\":<int 0-100>}, so "
"luong phan tu tuy y theo yeu cau nguoi dung. KHONG dung targetGroup/chance rieng le cho loai "
"trigger nay - PHAI dung dung dinh dang mang nay.\n\n"

"TARGETGROUP vs TAG - hai khai niem HOAN TOAN DOC LAP, KHONG duoc suy tu cai nay ra cai kia:\n"
"- \"targetGroup\": group SE BI trigger nay tac dong (chi dien khi trigger do co tham so nay "
"trong danh sach duoc xac nhan).\n"
"- \"tag\": tham so RIENG, CHUNG cho MOI loai trigger, dung de gan Group ID cho CHINH trigger "
"nay (de trigger khac nhu Spawn/Touch goi den no qua targetGroup cua chung). Chi dien \"tag\" "
"khi nguoi dung yeu cau ro rang (vd \"tag trigger nay la 50\", \"gan group id cho trigger de "
"spawn goi toi\"). Neu nguoi dung chi noi ve tag ma khong noi gi ve group bi tac dong, CHI dien "
"\"tag\", TUYET DOI KHONG tu dien them \"targetGroup\" hay nguoc lai.\n\n"

"Neu nguoi dung yeu cau thay doi/tang/giam MOT thong so cu the ma trigger do CO trong danh "
"sach tham so (vd \"tang do rung len\", \"bat che do vertex\", \"doi mau sang do\"), PHAI dua "
"chinh xac tham so tuong ung vao JSON voi gia tri moi - KHONG duoc bo qua no hay chi tra ve "
"targetGroup suong.\n\n"

+ ta::buildTrustedTriggerListForPrompt() + "\n\n"

"Danh sach tren la TOAN BO nhung gi ban duoc phep biet ve trigger/tham so - neu mot yeu cau "
"khong khop voi bat ky muc nao trong danh sach, dung TAO RA tham so hay id moi de dap ung, ma "
"chi lam trong pham vi da xac nhan.\n\n"

"Neu co object dang chon duoc cung cap, dung group cua no lam targetGroup mac dinh tru khi "
"nguoi dung noi khac. Khong giai thich gi them, chi tra ve JSON thuan.\n\n"
+ context + "Yeu cau: "
    ) + query;

    auto alive = m_alive;
    auto self = this;
    EditorUI* ui = m_editorUI;
    LevelEditorLayer* lel = m_editorLayer;

    ta::askGemini(prompt, alive, [alive, self, ui, lel, selected](bool ok, std::string const& textOrError) {
        if (!alive->load()) return;

        if (!ok) {
            self->appendLog(textOrError);
            return;
        }

        geode::log::info("TA: FloatingPanel generate raw: {}", textOrError);

        std::vector<GameObject*> created;
        int count = ta::spawnObjectsFromAIJson(ui, lel, selected, textOrError, &created);

        if (count > 0) {
            self->appendLog("Xong! Nan ra " + std::to_string(count) + " object, ngon lanh canh dao.");
            if (created.size() == 1) {
                ui->selectObject(created[0], false);
            } else if (!created.empty()) {
                auto arr = cocos2d::CCArray::create();
                for (auto obj : created) arr->addObject(obj);
                ui->selectObjects(arr, false);
            }
        } else {
            self->appendLog("Gemini no dinh cong, khong nan ra cai gi ca. Coi log ma bat loi.");
        }
    });
}

} // namespace ta