// ============================================================================
// EasingPreview.cpp
//
// Tinh nang: khi mo popup THAT cua GD de chinh Move Trigger / Rotate Trigger
// (SetupMoveCommandPopup / SetupRotateCommandPopup - ca 2 deu ke thua tu
// SetupTriggerPopup, xac nhan qua header binding that), nhet them 1 khung
// nho o goc popup, ben trong co 1 khoi vuong nho di chuyen qua lai lien tuc,
// dung DUNG easing/easing rate ma nguoi dung dang chon (m_easingType/
// m_easingRate - field THAT cua SetupTriggerPopup, doc truc tiep, khong can
// doan). Moi lan nguoi dung doi easing (hoac bat ky slider nao khac, vi GD
// goi chung 1 ham valueDidChange cho tat ca) thi khoi vuong tu dong doi
// kieu di chuyen theo, KHONG can bam Test Level.
//
// Field/class dung trong file nay DA VERIFY qua header binding that ban gui:
//   - SetupMoveCommandPopup, SetupRotateCommandPopup : public SetupTriggerPopup
//   - SetupTriggerPopup::m_easingType (EasingType), m_easingRate (float)
//   - SetupTriggerPopup::m_width, m_height (kich thuoc popup, dung float)
//   - FLAlertLayer::m_mainLayer (CCLayer* - lop cha cua SetupTriggerPopup,
//     goc toa do (0,0) o CHINH GIUA popup - quy uoc chuan cua cac popup GD)
//
// VI TRI khung preview: dat o goc TREN-PHAI cua popup, cach mep 1 doan =
// khung preview + margin. Do khong the chay game that de nhin truc tiep xem
// co bi de len nut/slider nao khong, ban BUILD THU va neu thay de len thi
// chi can doi 2 dong toa do o duoi (kPreviewOffsetX/Y) - da comment ro.
// ============================================================================

#include <Geode/modify/SetupMoveCommandPopup.hpp>
#include <Geode/modify/SetupRotateCommandPopup.hpp>
#include <Geode/binding/SetupTriggerPopup.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

namespace ta {

// Kich thuoc + vi tri khung preview (chinh o day neu bi de len UI khac).
static constexpr float kPreviewW = 90.f;
static constexpr float kPreviewH = 30.f;
static constexpr float kSquareSize = 12.f;
static constexpr float kTravelDistance = kPreviewW - kSquareSize - 10.f; // khoang di chuyen trong khung

// Tao 1 CCActionInterval MoveBy da boc easing dung theo m_easingType/m_easingRate
// cua SetupTriggerPopup - EasingType enum THEO DUNG THU TU CHUAN cua GD
// (cung quy uoc da dung trong TriggerSchema.cpp: 0-18), khop voi ho ham
// CCEase* co san trong chinh engine cocos2d (khong can lib ngoai).
static CCActionInterval* wrapEasing(CCActionInterval* inner, int easingType, float rate) {
    if (rate <= 0.f) rate = 2.f; // GD mac dinh 2.0 khi rate = 0
    switch (easingType) {
        case 1:  return CCEaseInOut::create(inner, rate);
        case 2:  return CCEaseIn::create(inner, rate);
        case 3:  return CCEaseOut::create(inner, rate);
        case 4:  return CCEaseElasticInOut::create(inner, rate);
        case 5:  return CCEaseElasticIn::create(inner, rate);
        case 6:  return CCEaseElasticOut::create(inner, rate);
        case 7:  return CCEaseBounceInOut::create(inner);
        case 8:  return CCEaseBounceIn::create(inner);
        case 9:  return CCEaseBounceOut::create(inner);
        case 10: return CCEaseExponentialInOut::create(inner);
        case 11: return CCEaseExponentialIn::create(inner);
        case 12: return CCEaseExponentialOut::create(inner);
        case 13: return CCEaseSineInOut::create(inner);
        case 14: return CCEaseSineIn::create(inner);
        case 15: return CCEaseSineOut::create(inner);
        case 16: return CCEaseBackInOut::create(inner);
        case 17: return CCEaseBackIn::create(inner);
        case 18: return CCEaseBackOut::create(inner);
        default: return inner; // 0 = None, di chuyen deu, khong easing
    }
}

// 2 tag rieng: 1 cho khung nen (de tim lai khi rebuild), 1 cho chinh khoi
// vuong (KHONG dung index 0 vi border cung la child, them truoc square).
static constexpr int kPreviewNodeTag = 0x7A50E123;
static constexpr int kPreviewSquareTag = 0x7A50E124;

static void rebuildPreviewAction(SetupTriggerPopup* self) {
    auto mainLayer = static_cast<FLAlertLayer*>(self)->m_mainLayer;
    if (!mainLayer || !mainLayer->getChildren()) return;

    CCNode* square = nullptr;
    for (int i = 0; i < mainLayer->getChildren()->count(); i++) {
        auto child = static_cast<CCNode*>(mainLayer->getChildren()->objectAtIndex(i));
        if (child && child->getTag() == kPreviewNodeTag && child->getChildren()) {
            for (int j = 0; j < child->getChildren()->count(); j++) {
                auto sub = static_cast<CCNode*>(child->getChildren()->objectAtIndex(j));
                if (sub && sub->getTag() == kPreviewSquareTag) {
                    square = sub;
                    break;
                }
            }
            break;
        }
    }
    if (!square) return;

    square->stopAllActions();
    float rate = self->m_easingRate;
    int easing = (int)self->m_easingType;

    auto goRight = CCMoveTo::create(0.9f, ccp(kTravelDistance, square->getPositionY()));
    auto goLeft  = CCMoveTo::create(0.9f, ccp(0.f, square->getPositionY()));
    auto seq = CCSequence::create(
        wrapEasing(goRight, easing, rate),
        wrapEasing(goLeft, easing, rate),
        nullptr
    );
    square->runAction(CCRepeatForever::create(seq));
}

// Khung nen KEO-THA duoc: nguoi dung tu do keo khung nay sang bat ky vi tri
// nao trong popup thay vi phai dua vao toa do dinh san (kMarginX/Y) - vi
// khong the chay game that de do dac chinh xac, keo tay se chinh xac hon
// nhieu. Chi ap dung cho chinh cai bg (khung nen), khong can lam gi voi
// border/square ben trong (chung la con cua bg, tu di theo khi bg di chuyen).
class DragPreviewBG : public CCLayerColor {
public:
    static DragPreviewBG* create(ccColor4B color, float w, float h) {
        auto ret = new DragPreviewBG();
        if (ret && ret->initWithColor(color, w, h)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void onEnter() override {
        CCLayerColor::onEnter();
        // Priority rat am = duoc uu tien nhan touch TRUOC cac slider/nut khac
        // cua popup (so nho hon = uu tien cao hon trong CCTouchDispatcher).
        // swallow = true de khi dang keo khong lam slider ben duoi phan ung
        // nham theo.
        CCDirector::sharedDirector()->getTouchDispatcher()
            ->addTargetedDelegate(this, -10000, true);
    }

    void onExit() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        CCLayerColor::onExit();
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        auto local = convertTouchToNodeSpace(touch);
        if (local.x < 0 || local.y < 0 || local.x > getContentSize().width || local.y > getContentSize().height) {
            return false; // ngoai khung -> khong nhan, tra lai touch cho UI khac
        }
        m_dragging = true;
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (!m_dragging || !getParent()) return;
        auto parent = getParent();
        auto curLocal = parent->convertToNodeSpace(touch->getLocation());
        auto prevLocal = parent->convertToNodeSpace(touch->getPreviousLocation());
        setPosition(getPosition() + (curLocal - prevLocal));
    }

    void ccTouchEnded(CCTouch*, CCEvent*) override { m_dragging = false; }
    void ccTouchCancelled(CCTouch*, CCEvent*) override { m_dragging = false; }

private:
    bool m_dragging = false;
};

static void buildPreviewWidget(SetupTriggerPopup* self) {
    auto mainLayer = static_cast<FLAlertLayer*>(self)->m_mainLayer;
    if (!mainLayer) return;

    // DA XAC NHAN QUA BUILD THAT: goc toa do (0,0) cua m_mainLayer nam o
    // GOC DUOI-TRAI cua popup (khong phai chinh giua nhu doan cu tuong -
    // bug cu khien khung de len vung "Move Time"). Truc x hop le: 0..m_width,
    // truc y hop le: 0..m_height. De dat o goc TREN-PHAI, lay canh phai/tren
    // cua popup roi tru di kich thuoc khung + margin.
    // SUA LOI LOGIC CU: py = height - H - marginY, nen marginY CANG NHO thi
    // khung CANG SAT DINH (cang len cao, cang tranh xa hang Move Y ben duoi) -
    // ban truoc dat marginY lon (34) nen day khung XUONG THAP hon, gay loi
    // "te hon truoc". Gio dat marginY that nho de ep sat mep tren/phai that.
    static constexpr float kMarginX = 4.f;
    static constexpr float kMarginY = 32.f;
    float px = self->m_width - kPreviewW - kMarginX;
    float py = self->m_height - kPreviewH - kMarginY;

    log::info("[EasingPreview] popup w={} h={} -> widget px={} py={} (right edge={}, top edge={})",
        self->m_width, self->m_height, px, py, px + kPreviewW, py + kPreviewH);

    auto bg = DragPreviewBG::create({0, 0, 0, 140}, kPreviewW, kPreviewH);
    bg->setPosition({px, py});
    bg->setTag(kPreviewNodeTag);
    bg->setZOrder(1000);
    mainLayer->addChild(bg);

    auto border = CCLayerColor::create({255, 255, 255, 60}, kPreviewW, 1.f);
    border->setPosition({0, 0});
    bg->addChild(border);
    auto border2 = CCLayerColor::create({255, 255, 255, 60}, kPreviewW, 1.f);
    border2->setPosition({0, kPreviewH - 1.f});
    bg->addChild(border2);

    auto square = CCLayerColor::create({255, 220, 60, 255}, kSquareSize, kSquareSize);
    square->setPosition({6.f, (kPreviewH - kSquareSize) / 2.f});
    square->setTag(kPreviewSquareTag);
    bg->addChild(square);

    rebuildPreviewAction(self);
}

} // namespace ta

class $modify(TAMoveEasingPreview, SetupMoveCommandPopup) {
    bool init(EffectGameObject* object, cocos2d::CCArray* objects) {
        if (!SetupMoveCommandPopup::init(object, objects)) return false;
        ta::buildPreviewWidget(this);
        return true;
    }

    void valueDidChange(int tag, float value) {
        SetupMoveCommandPopup::valueDidChange(tag, value);
        ta::rebuildPreviewAction(this);
    }
};

class $modify(TARotateEasingPreview, SetupRotateCommandPopup) {
    bool init(EffectGameObject* object, cocos2d::CCArray* objects) {
        if (!SetupRotateCommandPopup::init(object, objects)) return false;
        ta::buildPreviewWidget(this);
        return true;
    }

    void valueDidChange(int tag, float value) {
        SetupRotateCommandPopup::valueDidChange(tag, value);
        ta::rebuildPreviewAction(this);
    }
};