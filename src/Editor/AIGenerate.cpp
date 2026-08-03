#include "AIGenerate.hpp"
#include "ObjectIDs.hpp"
#include "PlacementAssist.hpp"
#include "../Knowledge/TriggerSchema.hpp"
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/GradientTriggerObject.hpp>
#include <Geode/binding/ForceBlockGameObject.hpp>
#include <Geode/binding/ItemTriggerGameObject.hpp>
#include <Geode/binding/ShaderGameObject.hpp>
#include <Geode/binding/CameraTriggerGameObject.hpp>
#include <Geode/binding/ChanceTriggerGameObject.hpp>
#include <Geode/binding/ChanceObject.hpp>
#include <Geode/binding/KeyframeGameObject.hpp>
#include <Geode/binding/SFXTriggerGameObject.hpp>
#include <Geode/binding/TimerTriggerGameObject.hpp>
#include <Geode/binding/ArtTriggerGameObject.hpp>
#include <Geode/binding/SongTriggerGameObject.hpp>
#include <Geode/binding/CheckpointGameObject.hpp>
#include <Geode/binding/AdvancedFollowTriggerObject.hpp>
#include <Geode/binding/KeyframeAnimTriggerObject.hpp>
#include <Geode/binding/SequenceTriggerGameObject.hpp>
#include <Geode/binding/EnterEffectObject.hpp>
#include <Geode/binding/CountTriggerGameObject.hpp>
#include <Geode/binding/UISettingsGameObject.hpp>
#include <Geode/loader/Log.hpp>
#include <matjson.hpp>
#include <unordered_set>

using namespace geode::prelude;

namespace ta {

namespace {
    // Gemini deo nghe loi: system prompt dan roi ma no van thich boc JSON
    // vao ```json ... ``` cho dep. Thoi ke lam cua no, tu tim '{' dau tien
    // toi '}' cuoi cung ma cat, khoi can nan ni.
    std::string stripCodeFence(std::string s) {
        auto start = s.find('{');
        auto end = s.rfind('}');
        if (start == std::string::npos || end == std::string::npos || end < start) return s;
        return s.substr(start, end - start + 1);
    }

    // ------------------------------------------------------------------
    // Day la danh sach TAT CA key JSON ma file nay THAT SU dung toi (qua
    // applyCommonParams / applyExtraTriggerParams / khoi gradient rieng /
    // "tag"). Ly do phai co cai nay: hoi truoc TriggerSchema.cpp khai la
    // "param hop le" nhung code ben duoi lai quen xu ly, the la gia tri
    // Gemini tra ve cu roi vao hu vo im lang, ngoi debug ca buoi moi phat
    // hien. Gio co warnUnappliedParams() bat loi do o duoi, dua vao set nay
    // ma so sanh.
    //
    // NHO: them key moi o dau trong file nay thi PHAI bo ten no vao day
    // luon, khong thoi warnUnappliedParams() no la lang keu "unapplied" oan
    // cho key ban vua lam, mat cong di tim lai.
    // ------------------------------------------------------------------
    const std::unordered_set<std::string>& appliedKeys() {
        static const std::unordered_set<std::string> keys = {
            // applyCommonParams
            "targetGroup", "centerGroup", "activate", "duration", "moveX", "moveY",
            "easing", "degrees", "scaleX", "scaleY", "opacity", "fadeIn", "hold",
            "fadeOut", "xModifier", "yModifier", "delay", "strength", "interval",
            "channel", "colorR", "colorG", "colorB", "blending", "zoom", "timeMod",
            "gravity", "gravityValue", "cameraEasing",
            "easingRate", "lockToPlayerX", "lockToPlayerY", "lockToCameraX",
            "lockToCameraY", "useMoveTarget", "moveTargetMode", "moveModX",
            "moveModY", "smallStep", "times360", "lockObjectRotation",
            "editCameraSettings", "disableGridSnap", "targetItemId",
            "item1", "item2",
            // Area/Enter Move/Rotate/Scale/Fade/Tint (10 trigger, chung class EnterEffectObject)
            "moveAngle", "moveDistance", "offset", "length",
            "easingInType", "easingInRate", "easingOutType", "easingOutRate",
            "xyMode", "deadzone", "twoDirections", "priority",
            "areaRotation", "areaScaleX", "areaScaleY", "fromOpacity",
            "toOpacity", "inbound", "tintChannelID", "areaTint", "hsvEnabled",
            // Camera Offset / GP Offset (suy luan, xem comment trong code)
            "offsetX", "offsetY",
            // Pickup Trigger / Instant Count Trigger
            "count", "targetCount", "comparisonType",
            "xRef", "yRef", "xRelative", "yRelative", "activateOnExactly",
            "pulseTargetType", "copyColorID", "copyOpacity", "pulseMainOnly",
            "pulseDetailOnly", "pulseExclusive",
            "positionXMod", "positionYMod", "rotationMod", "scaleXMod", "scaleYMod",
            "sequenceMode", "resetMode", "uniqueRemap",
            // art trigger (change bg/ground/midground) / song trigger / checkpoint
            "artIndex", "songChannel", "respawnID",
            // advanced follow trigger (35 field rieng, "delay"/"easing" o day
            // la field RIENG cua AdvancedFollowTriggerObject, khac field
            // chung cung ten o applyCommonParams)
            "startSpeed", "startSpeedReference", "startDirection",
            "startDirectionReference", "maxSpeed", "xOnly", "yOnly", "maxRange",
            "maxRangeReference", "acceleration", "steerForce",
            "steerForceLowEnabled", "steerForceLow", "steerForceHighEnabled",
            "steerForceHigh", "speedRangeLow", "speedRangeHigh", "breakForce",
            "breakAngle", "breakSteerForce", "breakSteerSpeedLimit",
            "targetDirection", "ignoreDisabled", "rotateDirection",
            "rotationOffset", "nearAcceleration", "nearDistance", "nearFriction",
            "friction", "rotateEasing", "rotateDeadZ", "priority", "followMode",
            "exclusive", "startMode",
            // gradient trigger (khoi rieng trong spawnObjectsFromAIJson)
            "gradientId", "vertexMode", "blendingMode", "blendingLayer", "disable",
            "groupTopLeft", "groupTopRight", "groupBottomRight", "groupBottomLeft",
            "groupUp", "groupLeft", "groupDown", "groupRight",
            // force block/circle
            "force", "minForce", "maxForce", "relativeForce", "forceRange", "forceID",
            // item edit trigger
            "targetItemId1", "targetItemId2", "item1Mode", "item2Mode",
            "targetItemMode", "mod1", "mod2", "signType1", "signType2",
            "resultType1", "resultType2", "resultType3", "roundType1", "roundType2",
            "tolerance", "persistent", "targetAll", "reset", "useTimer",
            // shader trigger (17 loai)
            "speed", "outer", "inner", "maxSize", "waveWidth", "targetX", "targetY",
            "screenOffsetX", "screenOffsetY", "invert", "rotate", "useX", "useY",
            "zLayerMin", "zLayerMax", "disableAll", "editorDisabled",
            // camera edge trigger
            "edgeDirection", "followObject", "followEasing", "smoothVelocity",
            "velocityModifier", "exitInstant", "exitStatic", "previewOpacity",
            // random/advanced random/sequence trigger
            "chanceList",
            // keyframe point
            "keyframeGroup", "keyframeIndex", "referenceOnly", "proximity", "curve",
            "closeLoop", "timeMode", "spawnDelay", "previewArt", "autoLayer",
            "direction", "revolutions", "lineOpacity",
            // sfx trigger
            "soundID", "pitch", "volume", "start", "end", "reverb", "loop",
            "unique", "sfxGroup", "stop",
            // timer trigger
            "startTime", "targetTime", "stopTimeEnabled", "dontOverride",
            "ignoreTimeWarp", "startPaused", "multiActivate", "controlType",
            // 3 tham so chung cho MOI trigger (m_isTouchTriggered/
            // m_isSpawnTriggered/m_isMultiTriggered tren EffectGameObject)
            "touchTriggered", "spawnTriggered", "multiTriggered",
            // meta - khong phai tham so trigger, xu ly rieng ben ngoai cac ham apply
            "id", "x", "y", "tag",
        };
        return keys;
    }

    // Kiem tra tung tham so ma schema cua trigger nay khai bao (tuc la AI
    // DUOC PHEP dung, xem TrustedTriggerList.cpp) - neu AI co tra ve key do
    // (item.contains) nhung key khong nam trong appliedKeys(), nghia la gia
    // tri se bi bo qua hoan toan khi ghi vao object. Chi log warning (khong
    // chan spawn) de khong lam vo tinh nang hien tai, nhung giup phat hien
    // ngay khi build/test thay vi debug im lang sau nay.
    void warnUnappliedParams(matjson::Value const& item, int objID) {
        const TriggerSchema* schema = nullptr;
        for (auto& s : triggerSchemas()) {
            if (s.gdObjectId == objID) { schema = &s; break; }
        }
        if (!schema) return;

        for (auto& p : schema->params) {
            if (item.contains(p.name) && !appliedKeys().count(p.name)) {
                geode::log::warn(
                    "TA: [AIGenerate] tham so '{}' cua trigger '{}' (objID={}) duoc AI "
                    "tra ve nhung KHONG co code nao ap dung no vao object - gia tri se "
                    "bi bo qua. Can bo sung xu ly trong AIGenerate.cpp (va them ten key "
                    "vao appliedKeys() sau khi lam).",
                    p.name, schema->id, objID
                );
            }
        }
    }

    int asIntOr(matjson::Value const& v, std::string const& key, int fallback) {
        if (!v.contains(key)) return fallback;
        auto r = v[key].asInt();
        return r.isOk() ? r.unwrap() : fallback;
    }

    double asDoubleOr(matjson::Value const& v, std::string const& key, double fallback) {
        if (!v.contains(key)) return fallback;
        auto r = v[key].asDouble();
        return r.isOk() ? r.unwrap() : fallback;
    }

     // Gemini doi khi tra ve bool duoi dang so nguyen 1/0 thay vi true/false
    // (vd "vertexMode": 1). matjson::asBool() KHONG tu ep kieu tu number
    // sang bool nen se fail va roi ve fallback sai. Thu asBool() truoc,
    // neu that bai thi thu doc nhu so nguyen va coi != 0 la true.
    bool asBoolOr(matjson::Value const& v, std::string const& key, bool fallback) {
        if (!v.contains(key)) return fallback;

        auto asBool = v[key].asBool();
        if (asBool.isOk()) return asBool.unwrap();

        auto asInt = v[key].asInt();
        if (asInt.isOk()) return asInt.unwrap() != 0;

        return fallback;
    }

    // GD luu m_moveOffset bang PIXEL that trong bo nho, nhung popup Move
    // Trigger hien thi cho nguoi choi bang "don vi" rieng, ty le CO DINH:
    // 10 don vi hien thi = 1 o luoi = 30 pixel  =>  1 don vi = 3 pixel.
    // Gemini duoc yeu cau tra ve moveX/moveY theo DON VI HIEN THI (giong
    // nguoi dung go tay vao popup), nen phai nhan 3 truoc khi gan vao
    // m_moveOffset - neu khong, "3" nguoi dung muon se bi luu thanh 3px
    // (hien ra la 1 don vi) thay vi 3 don vi (9px).
    constexpr float kMoveUnitToPixel = 3.f;

    // ------------------------------------------------------------------
    // Ap dung TAT CA tham so ma Gemini co the tra ve, khop CHINH XAC ten
    // field da dung trong TriggerSchema.cpp (va trong TrustedTriggerList
    // prompt) - PHAI dong bo voi ten param o do, khong duoc dat ten khac.
    // ------------------------------------------------------------------
    void applyCommonParams(matjson::Value const& item, GameObject* obj,
                            EffectGameObject* eff, int defaultGroup) {
        if (item.contains("targetGroup")) {
            eff->m_targetGroupID = asIntOr(item, "targetGroup", defaultGroup);
        } else if (defaultGroup > 0) {
            eff->m_targetGroupID = defaultGroup;
        }
        if (item.contains("centerGroup"))
            eff->m_centerGroupID = asIntOr(item, "centerGroup", 0);

        // "activate" la tham so BOOL dung cho Toggle Trigger (0=tat,1=bat) -
        // KHONG phai "activateGroup". Ten cu bi lech voi TriggerSchema,
        // khien Toggle Trigger Gemini tao ra luon o trang thai mac dinh.
        if (item.contains("activate"))
            eff->m_activateGroup = asBoolOr(item, "activate", true);

        if (item.contains("duration"))
            eff->m_duration = (float)asDoubleOr(item, "duration", 0.5);

        // Quy doi don vi hien thi (giong popup Move Trigger) sang pixel that.
        if (item.contains("moveX"))
            eff->m_moveOffset.x = (float)asDoubleOr(item, "moveX", 0.0) * kMoveUnitToPixel;
        if (item.contains("moveY"))
            eff->m_moveOffset.y = (float)asDoubleOr(item, "moveY", 0.0) * kMoveUnitToPixel;

        if (item.contains("easing"))
            eff->m_easingType = (decltype(eff->m_easingType))asIntOr(item, "easing", 0);
        if (item.contains("easingRate"))
            eff->m_easingRate = (float)asDoubleOr(item, "easingRate", 2.0);

        // Move Trigger - field bo sung (Lock to Player/Camera X/Y, Move
        // Target, Move Mod X/Y, Small Step) - verify qua EffectGameObject that.
        if (item.contains("lockToPlayerX"))
            eff->m_lockToPlayerX = asBoolOr(item, "lockToPlayerX", false);
        if (item.contains("lockToPlayerY"))
            eff->m_lockToPlayerY = asBoolOr(item, "lockToPlayerY", false);
        if (item.contains("lockToCameraX"))
            eff->m_lockToCameraX = asBoolOr(item, "lockToCameraX", false);
        if (item.contains("lockToCameraY"))
            eff->m_lockToCameraY = asBoolOr(item, "lockToCameraY", false);
        if (item.contains("useMoveTarget"))
            eff->m_useMoveTarget = asBoolOr(item, "useMoveTarget", false);
        if (item.contains("moveTargetMode"))
            eff->m_moveTargetMode = (decltype(eff->m_moveTargetMode))asIntOr(item, "moveTargetMode", 0);
        if (item.contains("moveModX"))
            eff->m_moveModX = (float)asDoubleOr(item, "moveModX", 1.0);
        if (item.contains("moveModY"))
            eff->m_moveModY = (float)asDoubleOr(item, "moveModY", 1.0);
        if (item.contains("smallStep"))
            eff->m_smallStep = asBoolOr(item, "smallStep", false);

        // Rotate Trigger - field bo sung (360 rotations count, lock object
        // rotation) - verify qua EffectGameObject that.
        if (item.contains("times360"))
            eff->m_times360 = asIntOr(item, "times360", 0);
        if (item.contains("lockObjectRotation"))
            eff->m_lockObjectRotation = asBoolOr(item, "lockObjectRotation", false);

        // Camera Static Trigger - field bo sung - verify qua EffectGameObject
        // that (m_cameraEditCameraSettings, m_cameraDisableGridSnap).
        if (item.contains("editCameraSettings"))
            eff->m_cameraEditCameraSettings = asBoolOr(item, "editCameraSettings", false);
        if (item.contains("disableGridSnap"))
            eff->m_cameraDisableGridSnap = asBoolOr(item, "disableGridSnap", false);

        // "targetItemId" - ten CHUNG trong schema cho cac trigger lien quan
        // Item ID (count/instant_count/pickup/timer/time_control/time_event/
        // persistent_item_setup) - tat ca deu dung chung field m_itemID tren
        // EffectGameObject (verify qua header that).
        if (item.contains("targetItemId"))
            eff->m_itemID = asIntOr(item, "targetItemId", 0);

        if (item.contains("degrees"))
            eff->m_rotationDegrees = (float)asDoubleOr(item, "degrees", 0.0);
        if (item.contains("scaleX"))
            obj->m_scaleX = (float)asDoubleOr(item, "scaleX", 1.0);
        if (item.contains("scaleY"))
            obj->m_scaleY = (float)asDoubleOr(item, "scaleY", 1.0);
        if (item.contains("opacity"))
            eff->m_opacity = (float)asDoubleOr(item, "opacity", 1.0);
        if (item.contains("fadeIn"))
            eff->m_fadeInDuration = (float)asDoubleOr(item, "fadeIn", 0.0);
        if (item.contains("hold"))
            eff->m_holdDuration = (float)asDoubleOr(item, "hold", 0.2);
        if (item.contains("fadeOut"))
            eff->m_fadeOutDuration = (float)asDoubleOr(item, "fadeOut", 0.2);
        if (item.contains("xModifier"))
            eff->m_followXMod = (float)asDoubleOr(item, "xModifier", 1.0);
        if (item.contains("yModifier"))
            eff->m_followYMod = (float)asDoubleOr(item, "yModifier", 1.0);
        if (item.contains("delay"))
            eff->m_spawnTriggerDelay = (float)asDoubleOr(item, "delay", 0.0);

        if (item.contains("strength"))
            eff->m_shakeStrength = (float)asDoubleOr(item, "strength", 5.0);
        if (item.contains("interval"))
            eff->m_shakeInterval = (float)asDoubleOr(item, "interval", 0.03);

        if (item.contains("channel"))
            eff->m_channelValue = asIntOr(item, "channel", 1000);
        if (item.contains("colorR") || item.contains("colorG") || item.contains("colorB")) {
            eff->m_triggerTargetColor = cocos2d::ccColor3B{
                (GLubyte)asIntOr(item, "colorR", 255),
                (GLubyte)asIntOr(item, "colorG", 255),
                (GLubyte)asIntOr(item, "colorB", 255)
            };
        }
        if (item.contains("blending"))
            eff->m_usesBlending = asBoolOr(item, "blending", false);

        if (item.contains("zoom"))
            eff->m_zoomValue = (float)asDoubleOr(item, "zoom", 1.0);
        if (item.contains("timeMod"))
            eff->m_timeWarpTimeMod = (float)asDoubleOr(item, "timeMod", 1.0);
        if (item.contains("gravity"))
            eff->m_gravityValue = (float)asDoubleOr(item, "gravity", -1.0);
        // "gravityValue" la ten tham so THAT trong TriggerSchema.cpp/prompt cho
        // gravity_trigger ("gravity" o tren la ten cu con sot lai) - phai doc
        // ca 2 de gia tri AI tra ve khong bi roi mat.
        if (item.contains("gravityValue"))
            eff->m_gravityValue = (float)asDoubleOr(item, "gravityValue", -1.0);

        if (item.contains("cameraEasing"))
            eff->m_cameraEasingValue = (float)asDoubleOr(item, "cameraEasing", 0.5);

        // Pulse Trigger - field verify qua header binding that tren
        // EffectGameObject (dung chung, khong can cast rieng).
        if (item.contains("pulseTargetType"))
            eff->m_pulseTargetType = asIntOr(item, "pulseTargetType", 0);
        if (item.contains("copyColorID"))
            eff->m_copyColorID = asIntOr(item, "copyColorID", 0);
        if (item.contains("copyOpacity"))
            eff->m_copyOpacity = asBoolOr(item, "copyOpacity", false);
        if (item.contains("pulseMainOnly"))
            eff->m_pulseMainOnly = asBoolOr(item, "pulseMainOnly", false);
        if (item.contains("pulseDetailOnly"))
            eff->m_pulseDetailOnly = asBoolOr(item, "pulseDetailOnly", false);
        if (item.contains("pulseExclusive"))
            eff->m_pulseExclusive = asBoolOr(item, "pulseExclusive", false);

        // Count Trigger - "activateOnExactly" trong schema thuc ra la
        // checkbox "Activate Group" that trong popup GD, khong phai 1 tinh
        // nang rieng ten "activate on exactly" (da kiem tra SetupCountTriggerPopup.hpp:
        // chi co m_activateGroup/m_multiActivate, khong co gi ten "Exactly"
        // ca) - map ve dung field that m_activateGroup tren EffectGameObject.
        if (item.contains("activateOnExactly"))
            eff->m_activateGroup = asBoolOr(item, "activateOnExactly", false);

        // 3 tham so chung cho MOI trigger (checkbox Touch/Spawn/Multi
        // Triggered trong popup that cua GD) - field da verify qua header
        // binding EffectGameObject.hpp: m_isTouchTriggered, m_isSpawnTriggered,
        // m_isMultiTriggered.
        if (item.contains("touchTriggered"))
            eff->m_isTouchTriggered = asBoolOr(item, "touchTriggered", false);
        if (item.contains("spawnTriggered"))
            eff->m_isSpawnTriggered = asBoolOr(item, "spawnTriggered", false);
        if (item.contains("multiTriggered"))
            eff->m_isMultiTriggered = asBoolOr(item, "multiTriggered", false);
    }

    // ========================================================================
    // KHOI BO SUNG - moi khoi typeinfo_cast sang dung class con that, gan tay
    // tung member - bat buoc phai lam vay vi cac field nay KHONG nam trong
    // EffectGameObject chung (khong co ham setter generic theo property key).
    //
    // Field name dung trong JSON = dung ten field da khai bao trong
    // TriggerSchema.cpp (params list) cua tung trigger, PHAI dong bo neu doi ten.
    // ========================================================================
    void applyExtraTriggerParams(matjson::Value const& item, GameObject* obj, int objID) {
        // ---- Change BG/Ground/Midground Trigger (ArtTriggerGameObject) ---
        // Field verify qua header binding that: m_artIndex.
        if (objID == object_ids::CHANGE_BG_TRIGGER ||
            objID == object_ids::CHANGE_GROUND_TRIGGER ||
            objID == object_ids::CHANGE_MIDGROUND_TRIGGER) {
            if (auto art = typeinfo_cast<ArtTriggerGameObject*>(obj)) {
                if (item.contains("artIndex"))
                    art->m_artIndex = asIntOr(item, "artIndex", 0);
            }
        }

        // ---- Song Trigger (SongTriggerGameObject) -------------------------
        // Field verify qua header binding that: m_songChannel.
        if (objID == object_ids::SONG_TRIGGER) {
            if (auto song = typeinfo_cast<SongTriggerGameObject*>(obj)) {
                if (item.contains("songChannel"))
                    song->m_songChannel = asIntOr(item, "songChannel", 0);
            }
        }

        // ---- Checkpoint Trigger (CheckpointGameObject) --------------------
        // Field verify qua header binding that: m_respawnID.
        if (objID == object_ids::CHECKPOINT) {
            if (auto cp = typeinfo_cast<CheckpointGameObject*>(obj)) {
                if (item.contains("respawnID"))
                    cp->m_respawnID = asIntOr(item, "respawnID", 0);
            }
        }

        // ---- Advanced Follow Trigger (AdvancedFollowTriggerObject) --------
        // TOAN BO field duoi day da verify TEN THAT qua header binding that
        // (AdvancedFollowTriggerObject.hpp) - truoc ban nay khong co dong nao
        // xu ly ca, nen trigger nay chi tao ra duoc gia tri mac dinh du AI
        // tra ve gi di nua. targetGroup/delay da duoc applyCommonParams gan
        // vao eff->m_targetGroupID / eff->m_spawnTriggerDelay o tren roi,
        // o day chi bo sung field RIENG cua AdvancedFollowTriggerObject
        // (bao gom ca "delay"/"easing" - trung ten voi key chung nhung khac
        // Y NGHIA/kieu du lieu voi field chung, nen phai gan lai o day).
        if (objID == object_ids::ADVANCED_FOLLOW_TRIGGER) {
            if (auto af = typeinfo_cast<AdvancedFollowTriggerObject*>(obj)) {
                if (item.contains("delay"))
                    af->m_delay = (float)asDoubleOr(item, "delay", 0.0);
                if (item.contains("startSpeed"))
                    af->m_startSpeed = (float)asDoubleOr(item, "startSpeed", 0.0);
                if (item.contains("startSpeedReference"))
                    af->m_startSpeedReference = asIntOr(item, "startSpeedReference", 0);
                if (item.contains("startDirection"))
                    af->m_startDirection = (float)asDoubleOr(item, "startDirection", 0.0);
                if (item.contains("startDirectionReference"))
                    af->m_startDirectionReference = asIntOr(item, "startDirectionReference", 0);
                if (item.contains("maxSpeed"))
                    af->m_maxSpeed = (float)asDoubleOr(item, "maxSpeed", 700.0);
                if (item.contains("xOnly"))
                    af->m_xOnly = asBoolOr(item, "xOnly", false);
                if (item.contains("yOnly"))
                    af->m_yOnly = asBoolOr(item, "yOnly", false);
                if (item.contains("maxRange"))
                    af->m_maxRange = (float)asDoubleOr(item, "maxRange", 0.0);
                if (item.contains("maxRangeReference"))
                    af->m_maxRangeReference = asIntOr(item, "maxRangeReference", 0);
                if (item.contains("acceleration"))
                    af->m_acceleration = (float)asDoubleOr(item, "acceleration", 700.0);
                if (item.contains("steerForce"))
                    af->m_steerForce = (float)asDoubleOr(item, "steerForce", 0.0);
                if (item.contains("steerForceLowEnabled"))
                    af->m_steerForceLowEnabled = asBoolOr(item, "steerForceLowEnabled", false);
                if (item.contains("steerForceLow"))
                    af->m_steerForceLow = (float)asDoubleOr(item, "steerForceLow", 0.0);
                if (item.contains("steerForceHighEnabled"))
                    af->m_steerForceHighEnabled = asBoolOr(item, "steerForceHighEnabled", false);
                if (item.contains("steerForceHigh"))
                    af->m_steerForceHigh = (float)asDoubleOr(item, "steerForceHigh", 0.0);
                if (item.contains("speedRangeLow"))
                    af->m_speedRangeLow = (float)asDoubleOr(item, "speedRangeLow", 0.0);
                if (item.contains("speedRangeHigh"))
                    af->m_speedRangeHigh = (float)asDoubleOr(item, "speedRangeHigh", 0.0);
                if (item.contains("breakForce"))
                    af->m_breakForce = (float)asDoubleOr(item, "breakForce", 0.0);
                if (item.contains("breakAngle"))
                    af->m_breakAngle = (float)asDoubleOr(item, "breakAngle", 0.0);
                if (item.contains("breakSteerForce"))
                    af->m_breakSteerForce = (float)asDoubleOr(item, "breakSteerForce", 0.0);
                if (item.contains("breakSteerSpeedLimit"))
                    af->m_breakSteerSpeedLimit = (float)asDoubleOr(item, "breakSteerSpeedLimit", 0.0);
                if (item.contains("targetDirection"))
                    af->m_targetDirection = asBoolOr(item, "targetDirection", false);
                if (item.contains("ignoreDisabled"))
                    af->m_ignoreDisabled = asBoolOr(item, "ignoreDisabled", false);
                if (item.contains("rotateDirection"))
                    af->m_rotateDirection = asBoolOr(item, "rotateDirection", false);
                if (item.contains("rotationOffset"))
                    af->m_rotationOffset = (float)asDoubleOr(item, "rotationOffset", 0.0);
                if (item.contains("nearAcceleration"))
                    af->m_nearAcceleration = (float)asDoubleOr(item, "nearAcceleration", 0.0);
                if (item.contains("nearDistance"))
                    af->m_nearDistance = (float)asDoubleOr(item, "nearDistance", 0.0);
                if (item.contains("nearFriction"))
                    af->m_nearFriction = (float)asDoubleOr(item, "nearFriction", 0.0);
                if (item.contains("friction"))
                    af->m_friction = (float)asDoubleOr(item, "friction", 0.0);
                // "easing" o day la m_easing (FLOAT rieng cua trigger nay,
                // dieu khien do min), KHONG phai m_easingType (enum 0-18)
                // ma applyCommonParams gan cho Move/Rotate/Scale - hai cai
                // khac hoan toan, gan doc lap khong lien quan gi nhau.
                if (item.contains("easing"))
                    af->m_easing = (float)asDoubleOr(item, "easing", 0.0);
                if (item.contains("rotateEasing"))
                    af->m_rotateEasing = (float)asDoubleOr(item, "rotateEasing", 0.0);
                if (item.contains("rotateDeadZ"))
                    af->m_rotateDeadZ = (float)asDoubleOr(item, "rotateDeadZ", 0.0);
                if (item.contains("priority"))
                    af->m_priority = asIntOr(item, "priority", 0);
                if (item.contains("followMode"))
                    af->m_followMode = asIntOr(item, "followMode", 0);
                if (item.contains("exclusive"))
                    af->m_exclusive = asBoolOr(item, "exclusive", false);
                if (item.contains("startMode"))
                    af->m_startMode = asIntOr(item, "startMode", 0);
            }
        }

        // ---- Keyframe Animation Trigger (KeyframeAnimTriggerObject) ------
        // "timeMod" o day la field RIENG cua trigger nay (m_timeMod, dieu
        // chinh toc do phat hoat hinh) - KHAC HOAN TOAN voi "timeMod" chung
        // ma applyCommonParams gan cho Time Warp Trigger (eff->m_timeWarpTimeMod).
        // Trung ten nhung khac field/y nghia nen phai gan lai o day.
        if (objID == object_ids::KEYFRAME_ANIMATION_TRIGGER) {
            if (auto ka = typeinfo_cast<KeyframeAnimTriggerObject*>(obj)) {
                if (item.contains("timeMod"))
                    ka->m_timeMod = (float)asDoubleOr(item, "timeMod", 1.0);
                if (item.contains("positionXMod"))
                    ka->m_positionXMod = (float)asDoubleOr(item, "positionXMod", 1.0);
                if (item.contains("positionYMod"))
                    ka->m_positionYMod = (float)asDoubleOr(item, "positionYMod", 1.0);
                if (item.contains("rotationMod"))
                    ka->m_rotationMod = (float)asDoubleOr(item, "rotationMod", 1.0);
                if (item.contains("scaleXMod"))
                    ka->m_scaleXMod = (float)asDoubleOr(item, "scaleXMod", 1.0);
                if (item.contains("scaleYMod"))
                    ka->m_scaleYMod = (float)asDoubleOr(item, "scaleYMod", 1.0);
            }
        }

        // ---- Area/Enter Move/Rotate/Scale/Fade/Tint Trigger (10 loai deu
        // dung chung 1 class that: EnterEffectObject - xac nhan qua chu ky
        // static create() cua CAC Setup*Popup.hpp tuong ung, TAT CA deu nhan
        // tham so EnterEffectObject*, khong phai ten rieng nhu doan truoc).
        // Truoc ban nay 0 field nao duoc ap dung ca 10 trigger nay - gio
        // ap dung day du field CHUNG (Area con dung areaRotation/Scale/Tint/
        // Fade + Move rieng, Enter cung dung field y het nhung khong co
        // offset/xyMode/deadzone/twoDirections/priority theo dung schema).
        if (objID == object_ids::AREA_MOVE_TRIGGER || objID == object_ids::ENTER_MOVE_TRIGGER ||
            objID == object_ids::AREA_ROTATE_TRIGGER || objID == object_ids::ENTER_ROTATE_TRIGGER ||
            objID == object_ids::AREA_SCALE_TRIGGER || objID == object_ids::ENTER_SCALE_TRIGGER ||
            objID == object_ids::AREA_FADE_TRIGGER || objID == object_ids::ENTER_FADE_TRIGGER ||
            objID == object_ids::AREA_TINT_TRIGGER || objID == object_ids::ENTER_TINT_TRIGGER) {
            if (auto en = typeinfo_cast<EnterEffectObject*>(obj)) {
                if (item.contains("moveAngle"))
                    en->m_moveAngle = asIntOr(item, "moveAngle", 0);
                if (item.contains("moveDistance"))
                    en->m_moveDistance = asIntOr(item, "moveDistance", 0);
                if (item.contains("offset"))
                    en->m_offset = asIntOr(item, "offset", 0);
                if (item.contains("length"))
                    en->m_length = asIntOr(item, "length", 0);
                if (item.contains("easingInType"))
                    en->m_easingInType = (EasingType)asIntOr(item, "easingInType", 0);
                if (item.contains("easingInRate"))
                    en->m_easingInRate = (float)asDoubleOr(item, "easingInRate", 2.0);
                if (item.contains("easingOutType"))
                    en->m_easingOutType = (EasingType)asIntOr(item, "easingOutType", 0);
                if (item.contains("easingOutRate"))
                    en->m_easingOutRate = (float)asDoubleOr(item, "easingOutRate", 2.0);
                if (item.contains("xyMode"))
                    en->m_xyMode = asBoolOr(item, "xyMode", false);
                if (item.contains("deadzone"))
                    en->m_deadzone = (float)asDoubleOr(item, "deadzone", 0.0);
                if (item.contains("twoDirections"))
                    en->m_twoDirections = asBoolOr(item, "twoDirections", false);
                if (item.contains("priority"))
                    en->m_priority = asIntOr(item, "priority", 0);
                if (item.contains("areaRotation"))
                    en->m_areaRotation = (float)asDoubleOr(item, "areaRotation", 0.0);
                if (item.contains("areaScaleX"))
                    en->m_areaScaleX = (float)asDoubleOr(item, "areaScaleX", 1.0);
                if (item.contains("areaScaleY"))
                    en->m_areaScaleY = (float)asDoubleOr(item, "areaScaleY", 1.0);
                if (item.contains("fromOpacity"))
                    en->m_fromOpacity = (float)asDoubleOr(item, "fromOpacity", 1.0);
                if (item.contains("toOpacity"))
                    en->m_toOpacity = (float)asDoubleOr(item, "toOpacity", 1.0);
                if (item.contains("inbound"))
                    en->m_inbound = asBoolOr(item, "inbound", false);
                if (item.contains("tintChannelID"))
                    en->m_tintChannelID = asIntOr(item, "tintChannelID", 0);
                if (item.contains("areaTint"))
                    en->m_areaTint = (float)asDoubleOr(item, "areaTint", 0.0);
                if (item.contains("hsvEnabled"))
                    en->m_hsvEnabled = asBoolOr(item, "hsvEnabled", false);
            }
        }

        // ---- Camera Offset / Camera GP Offset Trigger ---------------------
        // offsetX/offsetY KHONG co field rieng trong CameraTriggerGameObject
        // (da kiem tra het header - chi co exitStatic/followObject/
        // followEasing/edgeDirection/smoothVelocity/velocityModifier/
        // exitInstant/previewOpacity). SUY LUAN (chua 100% chac chan, popup
        // that SetupCameraOffsetTrigger dung slider kieu giong Move Trigger)
        // la trigger nay tai su dung eff->m_moveOffset ke thua tu
        // EffectGameObject - neu sau nay test in-game thay sai thi can sua
        // lai. KHONG nhan he so kMoveUnitToPixel vi chua ro co dung quy uoc
        // don vi hien thi giong Move Trigger hay khong.
        if (objID == object_ids::CAMERA_OFFSET_TRIGGER ||
            objID == object_ids::CAMERA_GP_OFFSET_TRIGGER) {
            if (auto eff2 = typeinfo_cast<EffectGameObject*>(obj)) {
                if (item.contains("offsetX"))
                    eff2->m_moveOffset.x = (float)asDoubleOr(item, "offsetX", 0.0);
                if (item.contains("offsetY"))
                    eff2->m_moveOffset.y = (float)asDoubleOr(item, "offsetY", 0.0);
            }
        }

        // ---- Pickup Trigger / Instant Count Trigger ------------------------
        // Ca 2 dung chung class CountTriggerGameObject (xac nhan qua chu ky
        // SetupInstantCountPopup::create(CountTriggerGameObject*, ...)).
        // m_pickupCount/m_pickupTriggerMode ten goc danh cho Pickup Trigger
        // nhung GD tai su dung cho ca Instant Count (targetCount/comparisonType)
        // - suy luan tu ten field khop dung y nghia, chua co xac nhan 100%
        // tuyet doi tu save-string mau.
        // ---- Pickup Trigger / Instant Count Trigger / Count Trigger -------
        // Ca 3 GIA DINH dung chung class CountTriggerGameObject. Instant
        // Count + Pickup da xac nhan qua chu ky ham tao popup that. Rieng
        // Count Trigger (COUNT_TRIGGER=1611) thi popup SetupCountTriggerPopup
        // nhan tham so kieu EffectGameObject* (lop CHA), khong khang dinh
        // 100% object thuc te la CountTriggerGameObject - neu suy doan sai,
        // typeinfo_cast se tra ve null va khoi if nay se KHONG lam gi ca
        // (an toan, khong crash, nhung se van "khong set duoc" nhu cu) - ban
        // nen test thu trong game de xac nhan.
        if (objID == object_ids::PICKUP_TRIGGER || objID == object_ids::INSTANT_COUNT_TRIGGER ||
            objID == object_ids::COUNT_TRIGGER) {
            if (auto ct = typeinfo_cast<CountTriggerGameObject*>(obj)) {
                if (item.contains("count"))
                    ct->m_pickupCount = asIntOr(item, "count", 0);
                if (item.contains("targetCount"))
                    ct->m_pickupCount = asIntOr(item, "targetCount", 0);
                if (item.contains("comparisonType"))
                    ct->m_pickupTriggerMode = asIntOr(item, "comparisonType", 0);
                if (item.contains("multiActivate"))
                    ct->m_multiActivate = asBoolOr(item, "multiActivate", false);
            }
        }

        // ---- UI Trigger (UISettingsGameObject) -----------------------------
        // Class that xac nhan qua header binding (KHONG phai "UITriggerGameObject"
        // nhu doan ban dau - GD dat ten la UISettingsGameObject). Field
        // m_xRef/m_yRef/m_xRelative/m_yRelative verify 100% qua header that.
        if (objID == object_ids::UI_TRIGGER) {
            if (auto ui = typeinfo_cast<UISettingsGameObject*>(obj)) {
                if (item.contains("xRef"))
                    ui->m_xRef = asIntOr(item, "xRef", 0);
                if (item.contains("yRef"))
                    ui->m_yRef = asIntOr(item, "yRef", 0);
                if (item.contains("xRelative"))
                    ui->m_xRelative = asBoolOr(item, "xRelative", false);
                if (item.contains("yRelative"))
                    ui->m_yRelative = asBoolOr(item, "yRelative", false);
            }
        }

        // ---- Force Block / Force Circle ---------------------------------
        if (objID == object_ids::FORCE_BLOCK || objID == object_ids::FORCE_CIRCLE) {
            if (auto force = typeinfo_cast<ForceBlockGameObject*>(obj)) {
                if (item.contains("force"))
                    force->m_force = (float)asDoubleOr(item, "force", 1.0);
                if (item.contains("minForce"))
                    force->m_minForce = (float)asDoubleOr(item, "minForce", 0.0);
                if (item.contains("maxForce"))
                    force->m_maxForce = (float)asDoubleOr(item, "maxForce", 10.0);
                if (item.contains("relativeForce"))
                    force->m_relativeForce = asBoolOr(item, "relativeForce", false);
                if (item.contains("forceRange"))
                    force->m_forceRange = asBoolOr(item, "forceRange", false);
                if (item.contains("forceID"))
                    force->m_forceID = asIntOr(item, "forceID", 0);
            }
        }

        // ---- Item Edit Trigger -------------------------------------------
        if (objID == object_ids::ITEM_EDIT_TRIGGER) {
            if (auto ie = typeinfo_cast<ItemTriggerGameObject*>(obj)) {
                if (item.contains("targetItemId1"))
                    ie->m_itemID = asIntOr(item, "targetItemId1", 0);
                if (item.contains("targetItemId2"))
                    ie->m_itemID2 = asIntOr(item, "targetItemId2", 0);
                // (khong con "resultItemId" nua - da xoa khoi schema, xem
                // ly do trong TriggerSchema.cpp)
                if (item.contains("item1Mode"))
                    ie->m_item1Mode = asIntOr(item, "item1Mode", 0);
                if (item.contains("item2Mode"))
                    ie->m_item2Mode = asIntOr(item, "item2Mode", 0);
                if (item.contains("targetItemMode"))
                    ie->m_targetItemMode = asIntOr(item, "targetItemMode", 0);
                if (item.contains("mod1"))
                    ie->m_mod1 = (float)asDoubleOr(item, "mod1", 1.0);
                if (item.contains("mod2"))
                    ie->m_mod2 = (float)asDoubleOr(item, "mod2", 1.0);
                if (item.contains("signType1"))
                    ie->m_signType1 = asIntOr(item, "signType1", 0);
                if (item.contains("signType2"))
                    ie->m_signType2 = asIntOr(item, "signType2", 0);
                if (item.contains("resultType1"))
                    ie->m_resultType1 = asIntOr(item, "resultType1", 0);
                if (item.contains("resultType2"))
                    ie->m_resultType2 = asIntOr(item, "resultType2", 0);
                if (item.contains("resultType3"))
                    ie->m_resultType3 = asIntOr(item, "resultType3", 0);
                if (item.contains("roundType1"))
                    ie->m_roundType1 = asIntOr(item, "roundType1", 0);
                if (item.contains("roundType2"))
                    ie->m_roundType2 = asIntOr(item, "roundType2", 0);
                if (item.contains("tolerance"))
                    ie->m_tolerance = (float)asDoubleOr(item, "tolerance", 0.0);
                if (item.contains("persistent"))
                    ie->m_persistent = asBoolOr(item, "persistent", false);
                if (item.contains("targetAll"))
                    ie->m_targetAll = asBoolOr(item, "targetAll", false);
                if (item.contains("reset"))
                    ie->m_reset = asBoolOr(item, "reset", false);
                if (item.contains("useTimer"))
                    ie->m_timer = asBoolOr(item, "useTimer", false);
            }
        }

        // ---- Item Compare Trigger: dung chung class ItemTriggerGameObject
        // voi Item Edit, chi khac ten tham so trong schema (item1/item2 thay
        // vi targetItemId1/2) - map ve dung field m_itemID/m_itemID2 that.
        // "comparisonType" CHUA xac dinh duoc member tuong ung, de trong.
        if (objID == object_ids::ITEM_COMPARE_TRIGGER) {
            if (auto ic = typeinfo_cast<ItemTriggerGameObject*>(obj)) {
                if (item.contains("item1"))
                    ic->m_itemID = asIntOr(item, "item1", 0);
                if (item.contains("item2"))
                    ic->m_itemID2 = asIntOr(item, "item2", 0);
                if (item.contains("tolerance"))
                    ic->m_tolerance = (float)asDoubleOr(item, "tolerance", 0.0);
                // if (item.contains("comparisonType")) ic->m_??? = ...; // TODO: chua ro member
            }
        }

        // ---- Shader Trigger (17 loai deu dung chung 1 class ShaderGameObject) --
        if (object_ids::isShaderTrigger(objID)) {
            if (auto sh = typeinfo_cast<ShaderGameObject*>(obj)) {
                if (item.contains("speed"))
                    sh->m_speed = (float)asDoubleOr(item, "speed", 1.0);
                if (item.contains("strength"))
                    sh->m_strength = (float)asDoubleOr(item, "strength", 1.0);
                if (item.contains("outer"))
                    sh->m_outer = (float)asDoubleOr(item, "outer", 200.0);
                if (item.contains("inner"))
                    sh->m_inner = (float)asDoubleOr(item, "inner", 0.0);
                if (item.contains("maxSize"))
                    sh->m_maxSize = (float)asDoubleOr(item, "maxSize", 500.0);
                if (item.contains("waveWidth"))
                    sh->m_waveWidth = (float)asDoubleOr(item, "waveWidth", 10.0);
                if (item.contains("targetX"))
                    sh->m_targetX = (float)asDoubleOr(item, "targetX", 0.0);
                if (item.contains("targetY"))
                    sh->m_targetY = (float)asDoubleOr(item, "targetY", 0.0);
                if (item.contains("fadeIn"))
                    sh->m_fadeIn = (float)asDoubleOr(item, "fadeIn", 0.0);
                if (item.contains("fadeOut"))
                    sh->m_fadeOut = (float)asDoubleOr(item, "fadeOut", 0.0);
                if (item.contains("screenOffsetX"))
                    sh->m_screenOffsetX = (float)asDoubleOr(item, "screenOffsetX", 0.0);
                if (item.contains("screenOffsetY"))
                    sh->m_screenOffsetY = (float)asDoubleOr(item, "screenOffsetY", 0.0);
                if (item.contains("invert"))
                    sh->m_invert = asBoolOr(item, "invert", true);
                if (item.contains("rotate"))
                    sh->m_rotate = asBoolOr(item, "rotate", false);
                if (item.contains("useX"))
                    sh->m_useX = asBoolOr(item, "useX", true);
                if (item.contains("useY"))
                    sh->m_useY = asBoolOr(item, "useY", false);
                if (item.contains("zLayerMin"))
                    sh->m_zLayerMin = asIntOr(item, "zLayerMin", 0);
                if (item.contains("zLayerMax"))
                    sh->m_zLayerMax = asIntOr(item, "zLayerMax", 0);
                if (item.contains("disableAll"))
                    sh->m_disableAll = asBoolOr(item, "disableAll", false);
                if (item.contains("editorDisabled"))
                    sh->m_editorDisabled = asBoolOr(item, "editorDisabled", false);
                // colorR/G/B cua edit_color_shader_trigger dung chung
                // m_triggerTargetColor tu EffectGameObject - applyCommonParams da
                // xu ly roi, khong can lam lai o day.
            }
        }

        // ---- Camera Edge Trigger ------------------------------------------
        if (objID == object_ids::CAMERA_EDGE_TRIGGER) {
            if (auto cam = typeinfo_cast<CameraTriggerGameObject*>(obj)) {
                if (item.contains("edgeDirection"))
                    cam->m_edgeDirection = asIntOr(item, "edgeDirection", 0);
                if (item.contains("followObject"))
                    cam->m_followObject = asBoolOr(item, "followObject", false);
                if (item.contains("followEasing"))
                    cam->m_followEasing = (float)asDoubleOr(item, "followEasing", 1.0);
                if (item.contains("smoothVelocity"))
                    cam->m_smoothVelocity = asBoolOr(item, "smoothVelocity", false);
                if (item.contains("velocityModifier"))
                    cam->m_velocityModifier = (float)asDoubleOr(item, "velocityModifier", 0.0);
                if (item.contains("exitInstant"))
                    cam->m_exitInstant = asBoolOr(item, "exitInstant", false);
                if (item.contains("exitStatic"))
                    cam->m_exitStatic = asBoolOr(item, "exitStatic", false);
                if (item.contains("previewOpacity"))
                    cam->m_previewOpacity = (float)asDoubleOr(item, "previewOpacity", 1.0);
            }
        }

        // ---- Random / Advanced Random / Sequence Trigger -------------------
        if (objID == object_ids::RANDOM_TRIGGER ||
            objID == object_ids::ADVANCED_RANDOM_TRIGGER ||
            objID == object_ids::SEQUENCE_TRIGGER) {
            if (auto chance = typeinfo_cast<ChanceTriggerGameObject*>(obj)) {
                if (item.contains("chanceList") && item["chanceList"].isArray()) {
                    auto listRes = item["chanceList"].asArray();
                    if (listRes) {
                        chance->m_chanceObjects.clear();
                        for (auto& entry : listRes.unwrap()) {
                            ChanceObject co;
                            // m_groupID/m_chance da verify qua header binding
                            // that (ChanceObject.hpp), khong con la TODO nua.
                            co.m_groupID = asIntOr(entry, "groupId", 0);
                            co.m_chance  = asIntOr(entry, "chance", 0);
                            chance->m_chanceObjects.push_back(co);
                        }
                    }
                }
            }
        }

        // ---- Sequence Trigger: field rieng ngoai chanceList (verify qua
        // SequenceTriggerGameObject.hpp that: m_sequenceMode, m_resetMode,
        // m_uniqueRemap).
        if (objID == object_ids::SEQUENCE_TRIGGER) {
            if (auto seq = typeinfo_cast<SequenceTriggerGameObject*>(obj)) {
                if (item.contains("sequenceMode"))
                    seq->m_sequenceMode = asIntOr(item, "sequenceMode", 0);
                if (item.contains("resetMode"))
                    seq->m_resetMode = asIntOr(item, "resetMode", 0);
                if (item.contains("uniqueRemap"))
                    seq->m_uniqueRemap = asBoolOr(item, "uniqueRemap", false);
                if (item.contains("reset"))
                    seq->m_reset = (float)asDoubleOr(item, "reset", 0.0);
            }
        }

        // ---- Keyframe Point -------------------------------------------------
        if (objID == object_ids::KEYFRAME_POINT) {
            if (auto kf = typeinfo_cast<KeyframeGameObject*>(obj)) {
                if (item.contains("keyframeGroup"))
                    kf->m_keyframeGroup = asIntOr(item, "keyframeGroup", 0);
                if (item.contains("keyframeIndex"))
                    kf->m_keyframeIndex = asIntOr(item, "keyframeIndex", 0);
                if (item.contains("referenceOnly"))
                    kf->m_referenceOnly = asBoolOr(item, "referenceOnly", false);
                if (item.contains("proximity"))
                    kf->m_proximity = asBoolOr(item, "proximity", false);
                if (item.contains("curve"))
                    kf->m_curve = asBoolOr(item, "curve", false);
                if (item.contains("closeLoop"))
                    kf->m_closeLoop = asBoolOr(item, "closeLoop", false);
                if (item.contains("timeMode"))
                    kf->m_timeMode = asIntOr(item, "timeMode", 0);
                if (item.contains("spawnDelay"))
                    kf->m_spawnDelay = (float)asDoubleOr(item, "spawnDelay", 0.0);
                if (item.contains("previewArt"))
                    kf->m_previewArt = asBoolOr(item, "previewArt", true);
                if (item.contains("autoLayer"))
                    kf->m_autoLayer = asBoolOr(item, "autoLayer", false);
                if (item.contains("direction"))
                    kf->m_direction = asIntOr(item, "direction", 0);
                if (item.contains("revolutions"))
                    kf->m_revolutions = asIntOr(item, "revolutions", 0);
                if (item.contains("lineOpacity"))
                    kf->m_lineOpacity = (float)asDoubleOr(item, "lineOpacity", 1.0);
            }
        }

        // ---- SFX Trigger ------------------------------------------------------
        // BUG CO SAN (khong phai do lan sua nay gay ra): dieu kien cu chi co
        // SFX_TRIGGER, thieu EDIT_SFX_TRIGGER - nen Edit SFX Trigger tao ra
        // luon la mac dinh du AI tra ve gi. Ca 2 dung chung 1 class that
        // SFXTriggerGameObject.
        if (objID == object_ids::SFX_TRIGGER || objID == object_ids::EDIT_SFX_TRIGGER) {
            if (auto sfx = typeinfo_cast<SFXTriggerGameObject*>(obj)) {
                if (item.contains("soundID"))
                    sfx->m_soundID = asIntOr(item, "soundID", 0);
                if (item.contains("pitch"))
                    sfx->m_pitch = (float)asDoubleOr(item, "pitch", 1.0);
                if (item.contains("speed"))
                    sfx->m_speed = asIntOr(item, "speed", 100);
                if (item.contains("volume"))
                    sfx->m_volume = (float)asDoubleOr(item, "volume", 1.0);
                if (item.contains("start"))
                    sfx->m_start = asIntOr(item, "start", 0);
                if (item.contains("fadeIn"))
                    sfx->m_fadeIn = asIntOr(item, "fadeIn", 0);
                if (item.contains("end"))
                    sfx->m_end = asIntOr(item, "end", 0);
                if (item.contains("fadeOut"))
                    sfx->m_fadeOut = asIntOr(item, "fadeOut", 0);
                if (item.contains("reverb"))
                    sfx->m_reverb = asBoolOr(item, "reverb", false);
                if (item.contains("loop"))
                    sfx->m_loop = asBoolOr(item, "loop", false);
                if (item.contains("unique"))
                    sfx->m_unique = asBoolOr(item, "unique", false);
                if (item.contains("sfxGroup"))
                    sfx->m_sfxGroup = asIntOr(item, "sfxGroup", 0);
                if (item.contains("stop"))
                    sfx->m_stop = asBoolOr(item, "stop", false);
            }
        }

        // ---- Timer Trigger (that, tao/khoi dong Timer Item) -------------------
        if (objID == object_ids::TIME_TRIGGER) {
            if (auto timer = typeinfo_cast<TimerTriggerGameObject*>(obj)) {
                if (item.contains("startTime"))
                    timer->m_startTime = asDoubleOr(item, "startTime", 0.0);
                if (item.contains("targetTime"))
                    timer->m_targetTime = asDoubleOr(item, "targetTime", 0.0);
                if (item.contains("stopTimeEnabled"))
                    timer->m_stopTimeEnabled = asBoolOr(item, "stopTimeEnabled", false);
                if (item.contains("dontOverride"))
                    timer->m_dontOverride = asBoolOr(item, "dontOverride", false);
                if (item.contains("ignoreTimeWarp"))
                    timer->m_ignoreTimeWarp = asBoolOr(item, "ignoreTimeWarp", false);
                if (item.contains("timeMod"))
                    timer->m_timeMod = (float)asDoubleOr(item, "timeMod", 1.0);
                if (item.contains("startPaused"))
                    timer->m_startPaused = asBoolOr(item, "startPaused", false);
                if (item.contains("multiActivate"))
                    timer->m_multiActivate = asBoolOr(item, "multiActivate", false);
                if (item.contains("controlType"))
                    timer->m_controlType = asIntOr(item, "controlType", 0);
            }
        }
    }
}

int spawnObjectsFromAIJson(EditorUI* ui, LevelEditorLayer* lel,
                            GameObject* selected, std::string const& rawText,
                            std::vector<GameObject*>* outCreated) {
    if (!ui || !lel) {
        geode::log::info("TA: AI generate - ui or lel null");
        return 0;
    }

    auto cleaned = stripCodeFence(rawText);
    auto parsed = matjson::parse(cleaned);
    if (!parsed) {
        geode::log::info("TA: AI generate - JSON parse failed. Raw: {}", cleaned);
        return 0;
    }

    auto root = parsed.unwrap();
    if (!root.contains("objects") || !root["objects"].isArray()) {
        geode::log::info("TA: AI generate - no 'objects' array in response");
        return 0;
    }

    auto arrRes = root["objects"].asArray();
    if (!arrRes) {
        geode::log::info("TA: AI generate - 'objects' is not an array");
        return 0;
    }

    float baseX = selected ? selected->getPositionX() : 0.f;
    float baseY = selected ? selected->getPositionY() : 0.f;
    int defaultGroup = 0;
    if (selected && selected->m_groupCount > 0) {
        defaultGroup = selected->m_groups->at(0);
    }

    // Tat PlacementAssist trong luc AI tu dat object: neu khong, tick
    // "Auto Group ID"/"Auto Tag ID"/"Copy First" dang bat (do nguoi dung
    // bat luc tu tay dat) se tu dong ghi de group/tag cua MOI object AI
    // tao ra, ke ca khi AI chi duoc yeu cau gan tag chu khong phai group.
    // AI tu lo group/tag/param cua no qua applyCommonParams roi.
    //
    // Dung RAII guard thay vi gan tay true/false: neu vong lap ben duoi
    // return som hoac nem loi vi ly do nao do, destructor cua guard van
    // chay va tra cho lai gia tri cu, tranh truong hop co bug lam cho
    // 3 toggle Auto Group/Auto Tag/Copy First bi "dong bang" mai mai sau
    // 1 lan AI generate that bai giua chung.
    SuppressPlacementAssistGuard suppressGuard;

    int spawned = 0;
    for (auto& item : arrRes.unwrap()) {
        if (!item.contains("id")) continue;
        int objID = asIntOr(item, "id", 0);
        if (objID <= 0) continue;

        // Chan luon neu Gemini bia ra id khong phai trigger hop le (hallucination
        // guard) - tranh spawn rac vao level cua nguoi dung.
        if (!object_ids::isTriggerID(objID)) {
            geode::log::info("TA: AI generate - id {} not a known trigger, skipping", objID);
            continue;
        }

        // Canh bao neu AI tra ve tham so ma schema cua trigger nay cong bo
        // la hop le nhung code ben duoi khong ap dung no vao object (xem
        // appliedKeys()/warnUnappliedParams() o dau file) - khong chan
        // spawn, chi log de phat hien lech schema/code som.
        warnUnappliedParams(item, objID);

        float offX = (float)asDoubleOr(item, "x", 30.0);
        float offY = (float)asDoubleOr(item, "y", 0.0);

        auto pos = CCPoint(baseX + offX, baseY + offY);
        // Goi qua ui->createObject (khong phai EditorUI::createObject truc
        // tiep) de di qua hook TAEditorUI::createObject - nhung vi
        // g_suppressPlacementAssist dang bat nen hook se KHONG tu dong
        // gan group/tag/copy-first de len object nay.
        auto obj = ui->createObject(objID, pos);
        if (!obj) {
            geode::log::info("TA: AI generate - createObject failed for id {}", objID);
            continue;
        }

        if (auto eff = typeinfo_cast<EffectGameObject*>(obj)) {
            applyCommonParams(item, obj, eff, defaultGroup);

            // ---- Gradient Trigger: field rieng, khong dung chung voi
            // move/rotate/scale. targetGroup o tren (m_targetGroupID) da
            // duoc gan dung - do la group se duoc phu gradient len. Phan
            // duoi day chi bo sung field dac thu (vertex mode + 4 group
            // goc/canh, blending, gradient id).
            if (objID == object_ids::GRADIENT_TRIGGER) {
                if (auto grad = typeinfo_cast<GradientTriggerObject*>(obj)) {
                    grad->m_gradientID   = asIntOr(item, "gradientId", 0);
                    grad->m_vertexMode   = asBoolOr(item, "vertexMode", false);
                    grad->m_blendingMode = asIntOr(item, "blendingMode", 0);
                    grad->m_blendingLayer = asIntOr(item, "blendingLayer", 0);
                    grad->m_disable = asBoolOr(item, "disable", false);
                    grad->m_disableAll = false;

                    if (grad->m_vertexMode) {
                        grad->m_upBottomLeftID    = asIntOr(item, "groupTopLeft", 0);
                        grad->m_leftTopLeftID     = asIntOr(item, "groupTopRight", 0);
                        grad->m_downBottomRightID = asIntOr(item, "groupBottomRight", 0);
                        grad->m_rightTopRightID   = asIntOr(item, "groupBottomLeft", 0);
                    } else {
                        grad->m_upBottomLeftID    = asIntOr(item, "groupUp", 0);
                        grad->m_leftTopLeftID     = asIntOr(item, "groupLeft", 0);
                        grad->m_downBottomRightID = asIntOr(item, "groupDown", 0);
                        grad->m_rightTopRightID   = asIntOr(item, "groupRight", 0);
                    }

                    geode::log::info(
                        "TA: gradient obj={} vertexMode={} up={} down={} left={} right={}",
                        (void*)obj, grad->m_vertexMode,
                        grad->m_upBottomLeftID, grad->m_downBottomRightID,
                        grad->m_leftTopLeftID, grad->m_rightTopRightID
                    );
                }
            }
        }

        // ---- Cac trigger dung class rieng (khong phai truong hop chung
        // cua EffectGameObject/applyCommonParams): Force Block/Circle, Item
        // Edit, Shader (17 loai), Camera Edge, Random/Advanced Random/
        // Sequence, Keyframe Point, SFX, Timer. Goi rieng, khong phu thuoc
        // vao viec obj co cast duoc sang EffectGameObject hay khong.
        applyExtraTriggerParams(item, obj, objID);

        // ------------------------------------------------------------------
        // TAG: Group ID cua CHINH cai trigger nay (de trigger khac goi den
        // no qua Spawn/Touch/...). HOAN TOAN doc lap voi targetGroup (group
        // BI trigger tac dong) - KHONG duoc suy ra tag tu targetGroupID nua.
        // Truoc day code cu tu dong lay targetGroupID lam Tag khi object
        // chua co group nao, gay nham lan "tag trigger" thanh "targetGroup"
        // (vd bat vertex mode xong mat tag, hoac tag tu nhien bi set trung
        // group bi tac dong du AI khong duoc yeu cau vay).
        // ------------------------------------------------------------------
        if (item.contains("tag")) {
            int tagGroup = asIntOr(item, "tag", 0);
            if (tagGroup > 0) {
                obj->addToGroup(tagGroup);
            }
        }

        // Refresh badge group ID ngay, vi addToGroup() khong tu ve lai so
        // tren sprite.
        if (obj->m_groupCount > 0) {
            ui->selectObject(obj, false);
            ui->updateGroupIDLabel();
        }

        if (outCreated) outCreated->push_back(obj);
        spawned++;
    }

    // PlacementAssist duoc bat lai tu dong khi suppressGuard ra khoi scope
    // ngay ben duoi (destructor cua SuppressPlacementAssistGuard), du ham
    // ket thuc binh thuong hay thoat som - khong can dong tay nua.

    geode::log::info("TA: AI generate - spawned {} objects", spawned);
    return spawned;
}

} // namespace ta

// ============================================================================
// VIEC CAN LAM THEM (chua the hoan thanh voi thong tin hien co):
// 1. Them cac constant con thieu trong ObjectIDs.hpp (neu chua co):
//    FORCE_BLOCK, FORCE_CIRCLE, ITEM_EDIT_TRIGGER, CAMERA_EDGE_TRIGGER,
//    RANDOM_TRIGGER, ADVANCED_RANDOM_TRIGGER, SEQUENCE_TRIGGER,
//    KEYFRAME_POINT, SFX_TRIGGER, TIME_TRIGGER, va ham helper
//    isShaderTrigger(int) tra ve true cho dai ID shader.
// 2. Xac nhan lai ten member that trong ChanceObject.hpp (m_groupID/m_chance
//    dang la doan, chua verify).
// 3. resultItemId cua item_edit_trigger van chua tim duoc member luu tru -
//    can dieu tra them.
// ============================================================================