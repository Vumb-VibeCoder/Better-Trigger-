// ExplainPopup.cpp
#include "ExplainPopup.hpp"
#include "../Knowledge/FormulaLibrary.hpp"
#include "../Knowledge/TriggerSchema.hpp"
#include "../Editor/AIGenerate.hpp"
#include "../Editor/TrustedTriggerList.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <thread>

using namespace geode::prelude;

namespace ta {

namespace {
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    std::string easingName(int e) {
        switch (e) {
            case 0: return "None";
            case 1: return "Ease In Out";
            case 2: return "Ease In";
            case 3: return "Ease Out";
            case 4: return "Elastic In Out";
            case 5: return "Elastic In";
            case 6: return "Elastic Out";
            case 7: return "Bounce In Out";
            case 8: return "Bounce In";
            case 9: return "Bounce Out";
            case 10: return "Exponential In Out";
            case 11: return "Exponential In";
            case 12: return "Exponential Out";
            case 13: return "Sine In Out";
            case 14: return "Sine In";
            case 15: return "Sine Out";
            case 16: return "Back In Out";
            case 17: return "Back In";
            case 18: return "Back Out";
            default: return "Easing #" + std::to_string(e);
        }
    }

    std::string formatParam(EffectGameObject* eff, GameObject* obj, TriggerParam const& p) {
        std::ostringstream out;
        out << "- " << p.name << ": ";

        if (p.name == "duration")        out << eff->m_duration << "s";
        else if (p.name == "targetGroup") out << eff->m_targetGroupID;
        else if (p.name == "centerGroup") out << eff->m_centerGroupID;
        else if (p.name == "easing")      out << easingName((int)eff->m_easingType);
        else if (p.name == "moveX")       out << eff->m_moveOffset.x;
        else if (p.name == "moveY")       out << eff->m_moveOffset.y;
        else if (p.name == "degrees")     out << eff->m_rotationDegrees;
        else if (p.name == "delay")       out << eff->m_spawnTriggerDelay << "s";
        else if (p.name == "opacity")     out << eff->m_opacity;
        else if (p.name == "fadeIn")      out << eff->m_fadeInDuration << "s";
        else if (p.name == "hold")        out << eff->m_holdDuration << "s";
        else if (p.name == "fadeOut")     out << eff->m_fadeOutDuration << "s";
        else if (p.name == "activate")    out << (eff->m_activateGroup ? "on" : "off");
        else if (p.name == "xModifier")   out << eff->m_followXMod;
        else if (p.name == "yModifier")   out << eff->m_followYMod;
        else if (p.name == "scaleX")      out << obj->m_scaleX;
        else if (p.name == "scaleY")      out << obj->m_scaleY;
        else if (p.name == "strength")    out << eff->m_shakeStrength;
        else if (p.name == "interval")    out << eff->m_shakeInterval;
        else if (p.name == "channel")     out << eff->m_channelValue;
        else if (p.name == "colorR")      out << (int)eff->m_triggerTargetColor.r;
        else if (p.name == "colorG")      out << (int)eff->m_triggerTargetColor.g;
        else if (p.name == "colorB")      out << (int)eff->m_triggerTargetColor.b;
        else if (p.name == "blending")    out << (eff->m_usesBlending ? "on" : "off");
        else if (p.name == "zoom")        out << eff->m_zoomValue;
        else if (p.name == "timeMod")     out << eff->m_timeWarpTimeMod;
        else if (p.name == "gravity")     out << eff->m_gravityValue;
        else if (p.name == "cameraEasing") out << eff->m_cameraEasingValue;
        else return "";

        return out.str();
    }

    // Gop nhieu dong trong lien tiep (\n\n\n...) thanh 1 dong duy nhat,
    // tranh khoang cach doan qua rong khi Gemini xuong dong doi de ngan
    // cach doan van.
    std::string collapseNewlines(std::string const& text) {
        std::string collapsed;
        bool lastWasNewline = false;
        for (char c : text) {
            if (c == '\n') {
                if (lastWasNewline) continue;
                lastWasNewline = true;
            } else {
                lastWasNewline = false;
            }
            collapsed += c;
        }
        return collapsed;
    }

    // ------------------------------------------------------------------
    // Highlight mau: Gemini duoc yeu cau boc chu can nhan manh trong tag
    // <r></r> (do - canh bao/loi thuong gap), <g></g> (xanh la - tham so
    // quan trong), <p></p> (tim - thuat ngu/khai niem can chu y).
    // ------------------------------------------------------------------
    struct ColorRun {
        std::string text;
        cocos2d::ccColor3B color;
    };

    std::vector<ColorRun> parseColorTags(std::string const& raw) {
        static const cocos2d::ccColor3B kWhite  { 255, 255, 255 };
        static const cocos2d::ccColor3B kRed    { 255, 90,  90  };
        static const cocos2d::ccColor3B kGreen  { 120, 230, 120 };
        static const cocos2d::ccColor3B kPurple { 200, 130, 255 };

        std::vector<ColorRun> runs;
        cocos2d::ccColor3B current = kWhite;
        std::string buf;
        size_t i = 0;

        auto flush = [&]() {
            if (!buf.empty()) {
                runs.push_back({ buf, current });
                buf.clear();
            }
        };

        while (i < raw.size()) {
            if (raw.compare(i, 3, "<r>") == 0)  { flush(); current = kRed;    i += 3; continue; }
            if (raw.compare(i, 4, "</r>") == 0) { flush(); current = kWhite;  i += 4; continue; }
            if (raw.compare(i, 3, "<g>") == 0)  { flush(); current = kGreen;  i += 3; continue; }
            if (raw.compare(i, 4, "</g>") == 0) { flush(); current = kWhite;  i += 4; continue; }
            if (raw.compare(i, 3, "<p>") == 0)  { flush(); current = kPurple; i += 3; continue; }
            if (raw.compare(i, 4, "</p>") == 0) { flush(); current = kWhite;  i += 4; continue; }
            buf += raw[i];
            i++;
        }
        flush();
        return runs;
    }

    struct Token {
        std::string word;
        cocos2d::ccColor3B color;
        bool isNewline = false;
    };

    std::vector<Token> tokenize(std::vector<ColorRun> const& runs) {
        std::vector<Token> tokens;
        for (auto& run : runs) {
            std::string cur;
            for (char c : run.text) {
                if (c == '\n') {
                    if (!cur.empty()) { tokens.push_back({ cur, run.color, false }); cur.clear(); }
                    tokens.push_back({ "", run.color, true });
                } else if (std::isspace((unsigned char)c)) {
                    if (!cur.empty()) { tokens.push_back({ cur, run.color, false }); cur.clear(); }
                } else {
                    cur += c;
                }
            }
            if (!cur.empty()) tokens.push_back({ cur, run.color, false });
        }
        return tokens;
    }

    // Model duoc chon vi day la ban thay the CHINH THUC, on dinh (khong
    // phai preview/experimental) cho dong Flash-Lite tinh den thoi diem
    // code nay duoc viet (07/2026). gemini-2.5-flash-lite dang bi loi
    // 404 tam thoi tu phia Google nen chuyen sang ban 3.1.
    constexpr auto kGeminiModel = "gemini-3.1-flash-lite";
}

ExplainPopup* ExplainPopup::create(GameObject* selected, EditorUI* ui, LevelEditorLayer* lel) {
    auto ret = new ExplainPopup();
    if (ret->init(selected, ui, lel)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

ExplainPopup::~ExplainPopup() {
    if (m_alive) {
        m_alive->store(false);
    }
}

std::string ExplainPopup::describeSelected(GameObject* obj) {
    if (!obj) return "";

    const TriggerSchema* schema = nullptr;
    for (auto& s : triggerSchemas()) {
        if (s.gdObjectId == obj->m_objectID) { schema = &s; break; }
    }
    if (!schema) return "";

    std::ostringstream out;
    out << schema->displayName << "\n" << schema->shortExplanation;

    auto eff = typeinfo_cast<EffectGameObject*>(obj);
    if (eff && !schema->params.empty()) {
        out << "\n\nThong so hien tai:\n";
        bool any = false;
        for (auto& p : schema->params) {
            auto line = formatParam(eff, obj, p);
            if (line.empty()) continue;
            out << line << "\n";
            any = true;
        }
        if (!any) out << "(khong doc duoc thong so)";
    }

    return out.str();
}

bool ExplainPopup::init(GameObject* selected, EditorUI* ui, LevelEditorLayer* lel) {
    if (!Popup::init(380.f, 300.f)) return false;

    m_alive = std::make_shared<std::atomic<bool>>(true);
    m_editorUI = ui;
    m_editorLayer = lel;
    m_selectedObj = selected;

    this->setTitle("Trigger Explainer");

   m_queryInput = TextInput::create(220.f, "vd: move, xoay, scale...", "chatFont.fnt");
    m_queryInput->setFilter(
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789 "
        ".,=_-+*/()[]{}!?:;'\"<>%$#@&|^~`\n"
    );
    m_queryInput->setCallback([this](std::string const& text) {
        this->onQueryChanged(text);
    });
    m_mainLayer->addChildAtPosition(m_queryInput, Anchor::Top, ccp(0, -45));

    // Ket qua duoc dat truc tiep vao mainLayer, KHONG dung ScrollLayer -
    // tung gay loi hien thi lech vi tri ra ca goc man hinh. Thay vao do,
    // showRichText() tu dong thu nho chu neu noi dung dai.
    m_resultLabel = CCLabelBMFont::create("Go ten trigger de xem giai thich.", "chatFont.fnt");
    m_resultLabel->setScale(.42f);
    m_resultLabel->setAlignment(cocos2d::kCCTextAlignmentCenter);
    m_mainLayer->addChildAtPosition(m_resultLabel, Anchor::Top, ccp(0, -100));

    auto aiSpr = ButtonSprite::create("Ask AI", "goldFont.fnt", "GJ_button_01.png", .8f);
    aiSpr->setScale(.75f);
    auto aiBtn = CCMenuItemSpriteExtra::create(aiSpr, this, menu_selector(ExplainPopup::onAskAI));
    m_buttonMenu->addChildAtPosition(aiBtn, Anchor::Bottom, ccp(-70, 25));

    auto genSpr = ButtonSprite::create("Generate", "goldFont.fnt", "GJ_button_02.png", .8f);
    genSpr->setScale(.75f);
    auto genBtn = CCMenuItemSpriteExtra::create(genSpr, this, menu_selector(ExplainPopup::onGenerateAI));
    m_buttonMenu->addChildAtPosition(genBtn, Anchor::Bottom, ccp(70, 25));

    if (auto desc = describeSelected(selected); !desc.empty()) {
        m_queryInput->setString("");
        showPlainText(desc);
    }

    return true;
}

void ExplainPopup::clearResultRich() {
    if (m_resultRich) {
        m_resultRich->removeFromParent();
        m_resultRich = nullptr;
    }
}

void ExplainPopup::showPlainText(std::string const& text) {
    clearResultRich();
    if (m_resultLabel) {
        m_resultLabel->setVisible(true);
        m_resultLabel->setString(text.c_str());
    }
}

void ExplainPopup::showRichText(std::string const& rawText) {
    if (m_resultLabel) m_resultLabel->setVisible(false);
    clearResultRich();

    std::string text = collapseNewlines(rawText);

    auto runs = parseColorTags(text);
    auto tokens = tokenize(runs);

    // Tu dong thu nho chu neu qua nhieu dong, de luon vua khung thay vi
    // tran ra ngoai popup (khong dung ScrollLayer - da gay loi truoc do).
    size_t roughLineCount = std::count(text.begin(), text.end(), '\n') + 1;
    float scale = 0.42f;
    if (roughLineCount > 8)       scale = 0.30f;
    else if (roughLineCount > 5)  scale = 0.36f;

    const float maxWidth = 320.f;
    const float lineHeight = scale * 38.f; // ti le voi scale, giu khoang cach dong hop ly
    const float spaceWidth = 6.f * (scale / 0.42f);

    std::vector<std::vector<Token>> lines;
    lines.emplace_back();
    float x = 0.f;

    for (auto& tok : tokens) {
        if (tok.isNewline) {
            lines.emplace_back();
            x = 0.f;
            continue;
        }
        auto tmp = CCLabelBMFont::create(tok.word.c_str(), "chatFont.fnt");
        float w = tmp->getContentSize().width * scale;

        if (x > 0.f && x + w > maxWidth) {
            lines.emplace_back();
            x = 0.f;
        }
        lines.back().push_back(tok);
        x += w + spaceWidth;
    }

    float totalTextHeight = lines.size() * lineHeight;

    auto container = CCNode::create();
    container->setAnchorPoint({0.5f, 1.f});

    for (size_t li = 0; li < lines.size(); li++) {
        float lineWidth = 0.f;
        for (auto& tok : lines[li]) {
            auto tmp = CCLabelBMFont::create(tok.word.c_str(), "chatFont.fnt");
            lineWidth += tmp->getContentSize().width * scale + spaceWidth;
        }
        if (lineWidth > 0.f) lineWidth -= spaceWidth;

        float lx = -lineWidth / 2.f;
        // Dong dau tien (li=0) nam NGAY TAI diem neo (y=0), cac dong sau
        // di XUONG DUOI dan (y am), khop voi anchorPoint {0.5, 1}.
        float ly = -(float)li * lineHeight;

        for (auto& tok : lines[li]) {
            auto lbl = CCLabelBMFont::create(tok.word.c_str(), "chatFont.fnt");
            lbl->setScale(scale);
            lbl->setAnchorPoint({0.f, 1.f});
            lbl->setColor(tok.color);
            lbl->setPosition({lx, ly});
            container->addChild(lbl);

            float w = lbl->getContentSize().width * scale;
            lx += w + spaceWidth;
        }
    }

    m_mainLayer->addChildAtPosition(container, Anchor::Top, ccp(0, -100));
    m_resultRich = container;
}

void ExplainPopup::onQueryChanged(std::string const& text) {
    auto query = toLower(text);

    if (query.empty()) {
        showPlainText("Go ten trigger de xem giai thich.");
        return;
    }

    for (auto& schema : triggerSchemas()) {
        bool match = toLower(schema.displayName).find(query) != std::string::npos ||
                     toLower(schema.id).find(query) != std::string::npos;
        if (!match) {
            for (auto& alias : schema.aliases) {
                if (toLower(alias).find(query) != std::string::npos) { match = true; break; }
            }
        }
        if (match) {
            showPlainText(schema.shortExplanation);
            return;
        }
    }

    for (auto& formula : formulaLibrary()) {
        if (toLower(formula.displayName).find(query) != std::string::npos ||
            toLower(formula.id).find(query) != std::string::npos) {
            showPlainText(formula.description);
            return;
        }
    }

    showPlainText("Khong tim thay trong du lieu co san. Bam \"Ask AI\" de hoi Gemini, hoac \"Generate\" de tao trigger.");
}

void ExplainPopup::onAskAI(cocos2d::CCObject*) {
    auto query = m_queryInput ? m_queryInput->getString() : "";
    if (query.empty()) {
        showPlainText("Nhap cau hoi truoc khi bam Ask AI.");
        return;
    }

    auto apiKey = Mod::get()->getSettingValue<std::string>("gemini-api-key");
    if (apiKey.empty()) {
        showPlainText("Chua nhap Gemini API Key trong Settings cua mod.");
        return;
    }

    showPlainText("Dang hoi AI...");

    matjson::Value part;
    part["text"] = std::string(
        "Ban la chuyen gia ve Trigger va co che Editor trong Geometry Dash (ban 2.2, ho tro "
        "cac trigger nhu Move, Rotate, Scale, Follow, Spawn, Toggle, Pulse, Alpha, Color, "
        "Area Move/Rotate/Scale/Fade/Tint, Item Edit, Count, Instant Count, Gradient, Keyframe, "
        "Reverse, Retarget, Random, Advanced Random, Time Control, Camera (Static/Edge/Offset/"
        "Rotate/Zoom/Mode/Guide), Shader triggers, v.v.).\n\n"
        "QUY TAC BAT BUOC:\n"
        "1. Chi tra loi ve trigger/co che thuc su ton tai trong GD. Neu khong chac chan ten "
        "trigger hoac tham so co dung khong, noi ro \"khong chac chan\" thay vi bia dat.\n"
        "2. Uu tien dung dung thuat ngu chinh thuc trong editor (Group ID, Target Group ID, "
        "Center Group, Easing, Duration, Item ID, Lock to Player X/Y...), khong dich sai nghia.\n"
        "3. Tra loi ngan gon, toi da 3-4 cau: (a) trigger nay dung de lam gi, (b) tham so quan "
        "trong nhat can quan tam, (c) 1 luu y thuc te hay gap loi (vi du: quen dat Center Group "
        "gay xoay/scale sai tam, quen easing gay giat cuc).\n"
        "4. Neu cau hoi lien quan toi ket hop nhieu trigger, giai thich ro thu tu hoat dong va "
        "loi thuong gap khi ket hop.\n"
        "5. Khong dua ra huong dan lien quan toi hack, gian lan, hoac bypass he thong.\n"
        "6. phai xuong dong nhieu neu khong chu se bi dai ra va khong doc duoc.\n"
        "7. phai giai thich tung thu trong trigger do de lam gi neu tang hay giam se nhu the nao.\n"
        "8. QUAN TRONG: viet cau tra loi hoan toan KHONG DAU (bo het dau tieng Viet), vi font "
        "hien thi trong game khong doc duoc ky tu co dau.\n"
        "9. HIGHLIGHT MAU: boc CANH BAO / loi thuong gap trong <r>...</r> (do), boc THAM SO "
        "QUAN TRONG (ten tham so, gia tri can chu y) trong <g>...</g> (xanh la), boc THUAT NGU "
        "/ KHAI NIEM can nguoi doc chu y trong <p>...</p> (tim). Chi dung 3 cap tag nay, khong "
        "dung markdown khac, khong long tag vao nhau, moi tag chi bao 1-3 tu ngan gon.\n\n"
        "Cau hoi: "
    ) + query;

    matjson::Value content;
    content["parts"] = matjson::Value(std::vector<matjson::Value>{ part });

    matjson::Value body;
    body["contents"] = matjson::Value(std::vector<matjson::Value>{ content });

    std::string url = std::string("https://generativelanguage.googleapis.com/v1beta/models/")
                     + kGeminiModel + ":generateContent";

    auto alive = m_alive;
    ExplainPopup* self = this;

    std::thread([alive, self, url, body, apiKey]() {
        web::WebRequest req;
        req.bodyJSON(body);
        req.header("Content-Type", "application/json");
        req.header("x-goog-api-key", apiKey);

        auto res = req.postSync(url);

        geode::queueInMainThread([alive, self, res = std::move(res)]() mutable {
            if (!alive->load()) return;

            if (!res.ok()) {
                std::string detail = "HTTP " + std::to_string(res.code());
                auto body = res.string();
                if (body.isOk() && !body.unwrap().empty()) {
                    detail += ": " + body.unwrap().substr(0, 200);
                } else if (!res.errorMessage().empty()) {
                    detail += " - " + std::string(res.errorMessage());
                }
                self->showPlainText(detail);
                return;
            }
            auto jsonRes = res.json();
            if (!jsonRes) {
                self->showPlainText("Khong doc duoc phan hoi tu Gemini.");
                return;
            }

            auto json = jsonRes.unwrap();
            auto text = json["candidates"][0]["content"]["parts"][0]["text"].asString();
            if (text.isOk()) {
                self->showRichText(text.unwrap());
            } else {
                self->showPlainText("Gemini khong tra ve noi dung hop le.");
            }
        });
    }).detach();
}

void ExplainPopup::onGenerateAI(cocos2d::CCObject*) {
    auto query = m_queryInput ? m_queryInput->getString() : "";
    if (query.empty()) {
        showPlainText("Nhap mo ta trigger truoc khi bam Generate.");
        return;
    }

    if (!m_editorUI || !m_editorLayer) {
        showPlainText("Khong tim thay editor de tao trigger.");
        return;
    }

    auto apiKey = Mod::get()->getSettingValue<std::string>("gemini-api-key");
    if (apiKey.empty()) {
        showPlainText("Chua nhap Gemini API Key trong Settings cua mod.");
        return;
    }

    showPlainText("Dang tao trigger...");

    std::string context;
    if (m_selectedObj) {
        context = "Object dang chon: ID " + std::to_string(m_selectedObj->m_objectID);
        if (m_selectedObj->m_groupCount > 0) {
            context += ", group " + std::to_string(m_selectedObj->m_groups->at(0));
        }
        context += ". Vi tri (" + std::to_string(m_selectedObj->getPositionX()) + ", "
                 + std::to_string(m_selectedObj->getPositionY()) + ").\n";
    }

    // QUAN TRONG: neu liet ke vi du JSON co field co dinh (targetGroup/
    // centerGroup/activateGroup/duration/moveX/moveY/easing) thi Gemini se
    // bam theo dung schema vi du duoc cho, KHONG tu suy ra them field ngoai
    // vi du (vd degrees/scaleX/scaleY/opacity/vertexMode/strength/interval/
    // channel/colorR/colorG/colorB/zoom/timeMod/gravity...) du
    // buildTrustedTriggerListForPrompt() co liet ke day du. Vi vay: bo vi du
    // field co dinh, chi dan dung ten tham so THEO DUNG LOAI TRIGGER trong
    // danh sach ben duoi.
    matjson::Value part;
    part["text"] = std::string(
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

    matjson::Value content;
    content["parts"] = matjson::Value(std::vector<matjson::Value>{ part });

    matjson::Value body;
    body["contents"] = matjson::Value(std::vector<matjson::Value>{ content });

    std::string url = std::string("https://generativelanguage.googleapis.com/v1beta/models/")
                     + kGeminiModel + ":generateContent";

    auto alive = m_alive;
    ExplainPopup* self = this;
    EditorUI* ui = m_editorUI;
    LevelEditorLayer* lel = m_editorLayer;
    GameObject* selected = m_selectedObj;

    std::thread([alive, self, url, body, apiKey, ui, lel, selected]() {
        web::WebRequest req;
        req.bodyJSON(body);
        req.header("Content-Type", "application/json");
        req.header("x-goog-api-key", apiKey);

        auto res = req.postSync(url);

        geode::queueInMainThread([alive, self, res = std::move(res), ui, lel, selected]() mutable {
            if (!alive->load()) return;

            if (!res.ok()) {
                std::string detail = "HTTP " + std::to_string(res.code());
                auto body = res.string();
                if (body.isOk() && !body.unwrap().empty()) {
                    detail += ": " + body.unwrap().substr(0, 200);
                } else if (!res.errorMessage().empty()) {
                    detail += " - " + std::string(res.errorMessage());
                }
                self->showPlainText(detail);
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                self->showPlainText("Khong doc duoc phan hoi tu Gemini.");
                return;
            }

            auto json = jsonRes.unwrap();
            auto text = json["candidates"][0]["content"]["parts"][0]["text"].asString();
            if (!text.isOk()) {
                self->showPlainText("Gemini khong tra ve noi dung hop le.");
                return;
            }

            geode::log::info("TA: Generate raw response: {}", text.unwrap());

            int count = ta::spawnObjectsFromAIJson(ui, lel, selected, text.unwrap());
            if (count > 0) {
                self->showPlainText("Da tao " + std::to_string(count) + " object.");
            } else {
                self->showPlainText("AI khong tao duoc object nao hop le. Xem log de debug.");
            }
        });
    }).detach();
}

} // namespace ta