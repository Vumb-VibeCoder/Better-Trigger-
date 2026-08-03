#include "TriggerSchema.hpp"
#include <algorithm>

// ============================================================================
// Bảng schema này đã được đối chiếu với bindings gốc geode-sdk/bindings,
// bản 2.2074. Các class đã verify trực tiếp bằng header thật:
//   EffectGameObject, ChanceTriggerGameObject, ChanceObject, ArtTriggerGameObject,
//   SequenceTriggerGameObject, TimerTriggerGameObject, EventLinkTrigger,
//   SFXTriggerGameObject, KeyframeAnimTriggerObject, AdvancedFollowTriggerObject,
//   CameraTriggerGameObject.
//
// CẬP NHẬT LẦN NÀY:
// - random_trigger / advanced_random_trigger / sequence_trigger: viết lại dùng
//   hasChanceList=true (danh sách ChanceEntry{groupId, chance} thay vì params
//   phẳng targetGroup/centerGroup/chance cũ - đã xác nhận qua ChanceTriggerGameObject
//   ::m_chanceObjects là gd::vector<ChanceObject> động, không phải 2 nhánh cố định).
// - advanced_follow_trigger: mở rộng gần full field thật từ
//   AdvancedFollowTriggerObject (vẫn bỏ bớt các field *Variance ít dùng để đỡ
//   phình UI - ghi rõ trong chú thích).
// - camera_edge_trigger: đổi followGroup -> followObject (đúng field name thật,
//   verify 100% qua CameraTriggerGameObject, KHÔNG còn field nào thiếu ở trigger này).
// - Timer/EventLink/SFX/KeyframeAnim: field đã xác minh đầy đủ nhưng CHƯA có
//   object ID thật -> vẫn để ngoài bảng, note ở cuối file, sẵn sàng thêm ngay
//   khi có ID.
// - 16/17 loai Shader Trigger (tru invert_color_trigger da co san): BO SUNG
//   4 field chung ke thua tu ShaderGameObject/EffectGameObject ma AIGenerate.cpp
//   da xu ly san nhung schema truoc do thieu khai bao: zLayerMin, zLayerMax,
//   disableAll, editorDisabled. Day la bo sung CHAC CHAN (suy ra truc tiep tu
//   code AIGenerate.cpp da gan cac field nay cho moi shader trigger qua
//   isShaderTrigger()), KHONG phai doan them field moi.
//
// FIX LẦN NÀY (quan trọng): Bản trước bị tách LÀM ĐÔI thành 2 vector table
// riêng biệt do lỗi merge - khối "CAMERA TRIGGERS" trở đi (camera_zoom_trigger,
// color_trigger, gradient_trigger, item_edit_trigger, end_trigger, song_trigger,
// teleport_trigger, bpm_trigger, v.v. - tổng ~40 entries) nằm NGOÀI hàm
// triggerSchemas(), khiến file không biên dịch được và các trigger đó không
// bao giờ được đưa vào bảng thật. Đã GỘP LẠI thành 1 vector table duy nhất,
// xóa cặp "}; return table; }" dư thừa ở giữa. Không có entry nào bị xóa hay
// đổi nội dung trong lúc gộp - chỉ sửa cấu trúc.
// ============================================================================

namespace ta {

std::vector<TriggerSchema>& triggerSchemas() {
    struct TempTriggerSchema {
        std::string id;
        std::string displayName;
        int gdObjectId;
        bool flag1;
        bool flag2;
        bool flag3;
        bool flag4;
        bool flag5;
        bool flag6;
        std::vector<TriggerParam> params;
        std::vector<std::string> aliases;
        std::string description;
    };

    static std::vector<TriggerSchema> table = []() {
        const std::vector<TempTriggerSchema> temp = {

        // ==================================================================
        // MOVE / ROTATE / SCALE / ALPHA
        // ==================================================================
        {
            "move_trigger", "Move Trigger", 901,
            true, false,
            true, false, false, false,
            {
                { "duration",       ParamType::Float,  0.0f, 60.0f, 0.5f },
                { "moveX",          ParamType::Float, -30000.f, 30000.f, 0.f },
                { "moveY",          ParamType::Float, -30000.f, 30000.f, 0.f },
                { "easing",         ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingRate",     ParamType::Float,  0.f, 20.f, 2.0f },
                { "targetGroup",    ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "lockToPlayerX",  ParamType::Bool, 0.f, 1.f, 0.f },
                { "lockToPlayerY",  ParamType::Bool, 0.f, 1.f, 0.f },
                { "lockToCameraX",  ParamType::Bool, 0.f, 1.f, 0.f },
                { "lockToCameraY",  ParamType::Bool, 0.f, 1.f, 0.f },
                { "useMoveTarget",  ParamType::Bool, 0.f, 1.f, 0.f },
                { "moveTargetMode", ParamType::Int,  0.f, 1.f, 0.f },
                { "moveModX",       ParamType::Float, -10.f, 10.f, 1.f },
                { "moveModY",       ParamType::Float, -10.f, 10.f, 1.f },
                { "smallStep",      ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "move", "di chuyen", "di chuyển" },
            "Di chuyen mot group theo truc X/Y trong mot khoang thoi gian, ho tro Move Target va "
            "Lock to Player/Camera X/Y."
        },
        {
            "rotate_trigger", "Rotate Trigger", 1346,
            true, false,
            false, false, true, false,
            {
                { "duration",           ParamType::Float, 0.0f, 60.0f, 0.5f },
                { "degrees",            ParamType::Float, -3600.f, 3600.f, 90.f },
                { "easing",             ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingRate",         ParamType::Float, 0.f, 20.f, 2.0f },
                { "targetGroup",        ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "centerGroup",        ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "times360",           ParamType::Int, 0.f, 10.f, 0.f },
                { "lockObjectRotation", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "rotate", "xoay", "quay" },
            "Xoay mot group quanh mot tam mot so do trong mot khoang thoi gian."
        },
        {
            "scale_trigger", "Scale Trigger", 2067,
            true, false,
            false, true, false, false,
            {
                { "duration",    ParamType::Float, 0.0f, 60.0f, 0.5f },
                { "scaleX",      ParamType::Float, 0.f, 10.f, 1.f },
                { "scaleY",      ParamType::Float, 0.f, 10.f, 1.f },
                { "easing",      ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingRate",  ParamType::Float, 0.f, 20.f, 2.0f },
                { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "centerGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
            },
            { "scale", "phong to", "thu nho", "to nho" },
            "Phong to hoac thu nho mot group theo truc X/Y quanh mot tam."
        },
        {
            "alpha_trigger", "Alpha Trigger", 1007,
            true, false,
            false, false, false, true,
            {
                { "duration",    ParamType::Float, 0.0f, 60.0f, 0.5f },
                { "opacity",     ParamType::Float, 0.f, 1.f, 1.f },
                { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
            },
            { "alpha", "mo dan", "an", "opacity" },
            "Thay doi do trong suot cua mot group trong mot khoang thoi gian."
        },
        {
            "toggle_trigger", "Toggle Trigger", 1049,
            false, true,
            false, false, false, false,
            {
                { "activate",    ParamType::Bool, 0.f, 1.f, 1.f },
                { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
            },
            { "toggle", "bat tat", "on off" },
            "Bat hoac tat mot group ngay lap tuc."
        },
        {
            "spawn_trigger", "Spawn Trigger", 1268,
            false, true,
            false, false, false, false,
            {
                { "delay",       ParamType::Float, 0.f, 60.f, 0.f },
                { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
            },
            { "spawn", "kich hoat" },
            "Kich hoat mot group khac ngay lap tuc hoac sau delay."
        },

        // ==================================================================
        // ADVANCED FOLLOW - da mo rong theo field that AdvancedFollowTriggerObject.
        // Bo qua cac field *Variance it dung (delayVariance, startSpeedVariance,
        // maxSpeedVariance, accelerationVariance, breakForceVariance...) va
        // property310-315/unk7fc (chua ro y nghia) de UI khong qua phinh to.
        // ==================================================================
        {
            "advanced_follow_trigger", "Advanced Follow Trigger", 3016,
            false, true,
            true, false, false, false,
            {
                { "targetGroup",        ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "delay",               ParamType::Float, 0.f, 10.f, 0.f },
                { "startSpeed",          ParamType::Float, 0.f, 2000.f, 0.f },
                { "startSpeedReference", ParamType::Int, 0.f, 3.f, 0.f },     // 0: gia tri co dinh, 1: min, 2: max...
                { "startDirection",      ParamType::Float, 0.f, 360.f, 0.f },
                { "startDirectionReference", ParamType::Int, 0.f, 3.f, 0.f },
                { "maxSpeed",            ParamType::Float, 0.f, 2000.f, 700.f },
                { "xOnly",               ParamType::Bool, 0.f, 1.f, 0.f },
                { "yOnly",               ParamType::Bool, 0.f, 1.f, 0.f },
                { "maxRange",            ParamType::Float, 0.f, 5000.f, 0.f },
                { "maxRangeReference",   ParamType::Int, 0.f, 3.f, 0.f },
                { "acceleration",        ParamType::Float, 0.f, 2000.f, 700.f },
                { "steerForce",          ParamType::Float, 0.f, 20.f, 0.f },
                { "steerForceLowEnabled",ParamType::Bool, 0.f, 1.f, 0.f },
                { "steerForceLow",       ParamType::Float, 0.f, 20.f, 0.f },
                { "steerForceHighEnabled",ParamType::Bool, 0.f, 1.f, 0.f },
                { "steerForceHigh",      ParamType::Float, 0.f, 20.f, 0.f },
                { "speedRangeLow",       ParamType::Float, 0.f, 2000.f, 0.f },
                { "speedRangeHigh",      ParamType::Float, 0.f, 2000.f, 0.f },
                { "breakForce",          ParamType::Float, 0.f, 20.f, 0.f },
                { "breakAngle",          ParamType::Float, 0.f, 360.f, 0.f },
                { "breakSteerForce",     ParamType::Float, 0.f, 20.f, 0.f },
                { "breakSteerSpeedLimit",ParamType::Float, 0.f, 2000.f, 0.f },
                { "targetDirection",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "ignoreDisabled",      ParamType::Bool, 0.f, 1.f, 0.f },
                { "rotateDirection",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "rotationOffset",      ParamType::Float, -360.f, 360.f, 0.f },
                { "nearAcceleration",    ParamType::Float, 0.f, 2000.f, 0.f },
                { "nearDistance",        ParamType::Float, 0.f, 2000.f, 0.f },
                { "nearFriction",        ParamType::Float, 0.f, 20.f, 0.f },
                { "friction",            ParamType::Float, 0.f, 20.f, 0.f },
                { "easing",              ParamType::Float, 0.f, 20.f, 0.f },
                { "rotateEasing",        ParamType::Float, 0.f, 20.f, 0.f },
                { "rotateDeadZ",         ParamType::Float, 0.f, 360.f, 0.f },
                { "priority",            ParamType::Int, 0.f, 999.f, 0.f },
                { "followMode",          ParamType::Int, 0.f, 5.f, 0.f },
                { "exclusive",           ParamType::Bool, 0.f, 1.f, 0.f },
                { "startMode",           ParamType::Int, 0.f, 3.f, 0.f },
            },
            { "follow", "advanced follow", "theo doi" },
            "Mo phong vat ly chuyen dong cua mot group bam theo group khac: toc do, gia toc, steer "
            "force (luc lai huong), break force (luc phanh), near-distance friction. Da map gan het "
            "field chinh tu AdvancedFollowTriggerObject that, tru mot vai field *Variance it dung."
        },
        {
            "pulse_trigger", "Pulse Trigger", 1006,
            false, true,
            false, false, false, false,
            {
                { "fadeIn",          ParamType::Float, 0.f, 10.f, 0.f },
                { "hold",            ParamType::Float, 0.f, 10.f, 0.2f },
                { "fadeOut",         ParamType::Float, 0.f, 10.f, 0.2f },
                { "targetGroup",     ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "colorR",          ParamType::Int, 0.f, 255.f, 255.f },
                { "colorG",          ParamType::Int, 0.f, 255.f, 255.f },
                { "colorB",          ParamType::Int, 0.f, 255.f, 255.f },
                { "pulseTargetType", ParamType::Int, 0.f, 1.f, 0.f },
                { "copyColorID",     ParamType::Int, 0.f, 1023.f, 0.f },
                { "copyOpacity",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "pulseMainOnly",   ParamType::Bool, 0.f, 1.f, 0.f },
                { "pulseDetailOnly", ParamType::Bool, 0.f, 1.f, 0.f },
                { "pulseExclusive",  ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "pulse", "nhap nhay", "beat" },
            "Tao hieu ung nhap nhay mau sac tuc thoi cho group hoac color channel, co the copy mau tu "
            "channel khac."
        },

        {
            "stop_trigger", "Stop Trigger", 1616,
            false, true,
            false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "stop", "dung lai" },
            "Dung ngay lap tuc mot trigger dang chay tren mot group."
        },
        {
            "follow_trigger", "Follow Trigger", 1347,
            false, true,
            true, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "follow don gian", "simple follow" },
            "Ban cu (khong phai Advanced Follow) - cho mot group bam theo chuyen dong cua group khac."
        },
        {
            "animate_trigger", "Animate Trigger", 1585,
            false, true,
            false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "animate", "hoat hinh" },
            "Kich hoat animation da dinh nghia san tren mot animated object."
        },
        {
            "follow_player_y_trigger", "Follow Player Y Trigger", 1814,
            false, true,
            true, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "follow player y", "bam theo nguoi choi" },
            "Cho mot group bam theo toa do Y cua nguoi choi."
        },
        {
            "edit_advanced_follow_trigger", "Edit Advanced Follow Trigger", 3660,
            false, true,
            false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "edit advanced follow" },
            "Chinh sua thong so cua mot Advanced Follow Trigger dang hoat dong."
        },
        {
            "retarget_advanced_follow_trigger", "Re-target Advanced Follow Trigger", 3661,
            false, true,
            false, false, false, false,
            {
                { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "centerGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
            },
            { "retarget", "doi doi tuong follow" },
            "Doi group ma mot Advanced Follow Trigger dang bam theo."
        },

        // ==================================================================
        // AREA TRIGGERS
        // ==================================================================
        {
            "area_move_trigger", "Area Move Trigger", 3006,
            false, false,
            true, false, false, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "moveX",         ParamType::Int, -1000.f, 1000.f, 0.f },
                { "moveY",         ParamType::Int, -1000.f, 1000.f, 0.f },
                { "moveAngle",     ParamType::Int, 0.f, 360.f, 0.f },
                { "moveDistance",  ParamType::Int, 0.f, 1000.f, 0.f },
                { "offset",        ParamType::Int, 0.f, 999.f, 0.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingInRate",  ParamType::Float, 0.f, 20.f, 2.f },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutRate", ParamType::Float, 0.f, 20.f, 2.f },
                { "xyMode",        ParamType::Bool, 0.f, 1.f, 0.f },
                { "deadzone",      ParamType::Float, 0.f, 1000.f, 0.f },
                { "twoDirections", ParamType::Bool, 0.f, 1.f, 0.f },
                { "priority",      ParamType::Int, 0.f, 999.f, 0.f },
            },
            { "area move" },
            "Di chuyen hang loat object trong mot vung voi hieu ung lan truyen mem (offset)."
        },
        {
            "area_rotate_trigger", "Area Rotate Trigger", 3007,
            false, false,
            false, false, true, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "areaRotation",  ParamType::Float, -360.f, 360.f, 0.f },
                { "offset",        ParamType::Int, 0.f, 999.f, 0.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingInRate",  ParamType::Float, 0.f, 20.f, 2.f },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutRate", ParamType::Float, 0.f, 20.f, 2.f },
                { "deadzone",      ParamType::Float, 0.f, 1000.f, 0.f },
                { "priority",      ParamType::Int, 0.f, 999.f, 0.f },
            },
            { "area rotate" },
            "Xoay hang loat object trong mot vung theo hieu ung lan truyen mem."
        },
        {
            "area_scale_trigger", "Area Scale Trigger", 3008,
            false, false,
            false, true, false, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "areaScaleX",    ParamType::Float, 0.f, 10.f, 1.f },
                { "areaScaleY",    ParamType::Float, 0.f, 10.f, 1.f },
                { "offset",        ParamType::Int, 0.f, 999.f, 0.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingInRate",  ParamType::Float, 0.f, 20.f, 2.f },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutRate", ParamType::Float, 0.f, 20.f, 2.f },
                { "priority",      ParamType::Int, 0.f, 999.f, 0.f },
            },
            { "area scale" },
            "Phong to/thu nho hang loat object trong mot vung theo hieu ung lan truyen mem."
        },
        {
            "area_fade_trigger", "Area Fade Trigger", 3009,
            false, false,
            false, false, false, true,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "fromOpacity",   ParamType::Float, 0.f, 1.f, 1.f },
                { "toOpacity",     ParamType::Float, 0.f, 1.f, 0.f },
                { "offset",        ParamType::Int, 0.f, 999.f, 0.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "inbound",       ParamType::Bool, 0.f, 1.f, 0.f },
                { "priority",      ParamType::Int, 0.f, 999.f, 0.f },
            },
            { "area fade" },
            "Mo dan/an dan hang loat object trong mot vung theo hieu ung lan truyen mem."
        },
        {
            "area_tint_trigger", "Area Tint Trigger", 3010,
            false, false,
            false, false, false, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "tintChannelID", ParamType::Int, 0.f, 1023.f, 1000.f },
                { "areaTint",      ParamType::Float, 0.f, 1.f, 1.f },
                { "offset",        ParamType::Int, 0.f, 999.f, 0.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "hsvEnabled",    ParamType::Bool, 0.f, 1.f, 0.f },
                { "priority",      ParamType::Int, 0.f, 999.f, 0.f },
            },
            { "area tint", "doi mau vung" },
            "Doi mau hang loat object trong mot vung theo hieu ung lan truyen mem."
        },
        {
            "edit_area_move_trigger", "Edit Area Move Trigger", 3011,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "edit area move" },
            "Chinh sua thong so cua mot Area Move Trigger dang chay."
        },
        {
            "edit_area_rotate_trigger", "Edit Area Rotate Trigger", 3012,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "edit area rotate" },
            "Chinh sua thong so cua mot Area Rotate Trigger dang chay."
        },
        {
            "edit_area_scale_trigger", "Edit Area Scale Trigger", 3013,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "edit area scale" },
            "Chinh sua thong so cua mot Area Scale Trigger dang chay."
        },
        {
            "edit_area_fade_trigger", "Edit Area Fade Trigger", 3014,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "edit area fade" },
            "Chinh sua thong so cua mot Area Fade Trigger dang chay."
        },
        {
            "edit_area_tint_trigger", "Edit Area Tint Trigger", 3015,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "edit area tint" },
            "Chinh sua thong so cua mot Area Tint Trigger dang chay."
        },


        {
    "change_bg_trigger", "Change BG Trigger", 3029,
    false, true, false, false, false, false,
    {
        { "artIndex", ParamType::Int, 0.f, 999.f, 0.f }, // key=533, xac nhan dung, rieng biet voi ground/mid
    },
    { "change bg", "doi nen" },
    "DA XAC NHAN DUNG. ID rieng, tach biet voi change_ground/change_midground, co field that "
    "duy nhat artIndex (key=533) de chon chi so bo art cho lop background."
},
{
    "change_ground_trigger", "Change Ground Trigger", 3030,
    false, true, false, false, false, false,
    {
        { "artIndex", ParamType::Int, 0.f, 999.f, 0.f }, // key=533
    },
    { "change ground", "doi mat dat" },
    "DA XAC NHAN DUNG. ID rieng, tach biet voi change_bg/change_midground, co field that "
    "duy nhat artIndex (key=533) de chon chi so bo art cho lop mat dat."
},
{
    "change_midground_trigger", "Change Middleground Trigger", 3031,
    false, true, false, false, false, false,
    {
        { "artIndex", ParamType::Int, 0.f, 999.f, 0.f }, // key=533
    },
    { "change middleground", "doi lop giua" },
    "DA XAC NHAN DUNG. ID rieng, tach biet voi change_bg/change_ground, co field that "
    "duy nhat artIndex (key=533) de chon chi so bo art cho lop giua."
},
        {
            "touch_trigger", "Touch Trigger", 1595,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "touch", "cham man hinh" },
            "Kich hoat group khi nguoi choi cham/giu/tha man hinh."
        },

        // ==================================================================
        // COUNT / PICKUP / ITEM
        // ==================================================================
        {
            "count_trigger", "Count Trigger", 1611,
            false, true, false, false, false, false,
            {
                { "targetGroup",       ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "targetItemId",      ParamType::Int, 0.f, 9999.f, 0.f },
                { "targetCount",       ParamType::Int, -9999.f, 9999.f, 0.f },
                { "activateOnExactly", ParamType::Bool, 0.f, 1.f, 0.f },
                { "multiActivate",     ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "count", "dem item" },
            "Kich hoat group khi mot Item ID dat gia tri dem nhat dinh."
        },
        {
            "instant_count_trigger", "Instant Count Trigger", 1811,
            false, true, false, false, false, false,
            {
                { "targetGroup",     ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "targetItemId",    ParamType::Int, 0.f, 9999.f, 0.f },
                { "targetCount",     ParamType::Int, -9999.f, 9999.f, 0.f },
                { "comparisonType",  ParamType::Int, 0.f, 2.f, 1.f },
            },
            { "instant count" },
            "Kiem tra gia tri Item ID ngay lap tuc va kich hoat group tuong ung."
        },
        {
            "pickup_trigger", "Pickup Trigger", 1817,
            false, true, false, false, false, false,
            {
                { "targetItemId",  ParamType::Int, 0.f, 9999.f, 0.f },
                { "count",         ParamType::Int, -999.f, 999.f, 1.f },
                { "multiActivate", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "pickup", "cong diem", "item" },
            "Tang/giam gia tri cua mot Item ID ngay lap tuc khi kich hoat."
        },

        // ---- RANDOM / SEQUENCE - viet lai theo ChanceTriggerGameObject that ----
{
    "random_trigger", "Random Trigger", 1912,
    false, true, false, false, false, false,
    {
        { "chanceList", ParamType::ChanceList, 0.f, 100.f, 0.f }, // danh sach {groupId, chance%}, dong
    },
    { "random", "ngau nhien" },
    "Kich hoat ngau nhien 1 trong N group, moi group co trong so (chance %) rieng. "
    "Danh sach {groupId, chance} co so luong dong tuy y (xem ParamType::ChanceList)."
},
{
    "advanced_random_trigger", "Advanced Random Trigger", 2068,
    false, true, false, false, false, false,
    {
        { "chanceList", ParamType::ChanceList, 0.f, 100.f, 0.f }, // danh sach {groupId, chance%}, dong
    },
    { "advanced random", "random nang cao" },
    "Ve mat cau truc du lieu giong het random_trigger (cung ke thua ChanceTriggerGameObject) - "
    "kich hoat ngau nhien 1 trong N group theo trong so. Danh sach {groupId, chance} co so "
    "luong dong tuy y (xem ParamType::ChanceList)."
},
{
    "sequence_trigger", "Sequence Trigger", 3607,
    false, true, false, false, false, false,
    {
        { "sequenceMode", ParamType::Int, 0.f, 3.f, 0.f },  // SequenceTriggerGameObject::m_sequenceMode
        { "resetMode",    ParamType::Int, 0.f, 3.f, 0.f },  // m_resetMode
        { "reset",        ParamType::Float, 0.f, 60.f, 0.f }, // m_reset
        { "uniqueRemap",  ParamType::Bool, 0.f, 1.f, 0.f },   // m_uniqueRemap
        { "chanceList",   ParamType::ChanceList, 0.f, 100.f, 0.f }, // danh sach {groupId, chance%}, dong
    },
    { "sequence", "chuoi tuan tu" },
    "Kich hoat lan luot nhieu group theo danh sach (ke thua ChanceTriggerGameObject), moi lan "
    "chay group tiep theo trong chuoi. sequenceMode dieu khien kieu lap (loop/random/...), "
    "resetMode+reset dieu khien khi nao chuoi quay lai dau. Danh sach {groupId, chance} co so "
    "luong dong tuy y (xem ParamType::ChanceList)."
},

        // ==================================================================
        // ENTER TRIGGERS - MOI phat hien (objects.csv). Dung chung class
        // EnterEffectObject voi Area Trigger nhung la object ID KHAC, hieu
        // ung ap dung khi object "xuat hien" (enter) thay vi lan truyen theo
        // vung (area). Field du doan giong Area (chua verify rieng field nao
        // chi dung cho Enter, co the trung phan lon voi Area).
        // ==================================================================
        {
            "enter_move_trigger", "Enter Move Trigger", 3017,
            false, false, true, false, false, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "moveX",         ParamType::Int, -1000.f, 1000.f, 0.f },
                { "moveY",         ParamType::Int, -1000.f, 1000.f, 0.f },
                { "moveAngle",     ParamType::Int, 0.f, 360.f, 0.f },
                { "moveDistance",  ParamType::Int, 0.f, 1000.f, 0.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingInRate",  ParamType::Float, 0.f, 20.f, 2.f },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutRate", ParamType::Float, 0.f, 20.f, 2.f },
            },
            { "enter move" },
            "[MOI - chua verify sau field, doan tu EnterEffectObject] Hieu ung di chuyen ap dung khi mot "
            "group 'xuat hien' lan dau trong man hinh, KHAC voi Area Move Trigger (khong co offset lan truyen)."
        },
        {
            "enter_rotate_trigger", "Enter Rotate Trigger", 3018,
            false, false, false, false, true, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "areaRotation",  ParamType::Float, -360.f, 360.f, 0.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
            },
            { "enter rotate" },
            "[MOI - chua verify sau field] Hieu ung xoay ap dung khi group xuat hien lan dau."
        },
        {
            "enter_scale_trigger", "Enter Scale Trigger", 3019,
            false, false, false, true, false, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "areaScaleX",    ParamType::Float, 0.f, 10.f, 1.f },
                { "areaScaleY",    ParamType::Float, 0.f, 10.f, 1.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
            },
            { "enter scale" },
            "[MOI - chua verify sau field] Hieu ung phong to/thu nho khi group xuat hien lan dau."
        },
        {
            "enter_fade_trigger", "Enter Fade Trigger", 3020,
            false, false, false, false, false, true,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "fromOpacity",   ParamType::Float, 0.f, 1.f, 0.f },
                { "toOpacity",     ParamType::Float, 0.f, 1.f, 1.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
            },
            { "enter fade" },
            "[MOI - chua verify sau field] Hieu ung mo/an dan khi group xuat hien lan dau (day chinh la co "
            "che 'Fade In' pho bien khi tao object moi trong editor)."
        },
        {
            "enter_tint_trigger", "Enter Tint Trigger", 3021,
            false, false, false, false, false, false,
            {
                { "targetGroup",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "tintChannelID", ParamType::Int, 0.f, 1023.f, 1000.f },
                { "areaTint",      ParamType::Float, 0.f, 1.f, 1.f },
                { "length",        ParamType::Int, 0.f, 999.f, 60.f },
                { "easingInType",  ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
                { "easingOutType", ParamType::Easing, 0.f, 18.f, (float)(int)Easing::EaseInOut },
            },
            { "enter tint" },
            "[MOI - chua verify sau field] Hieu ung doi mau khi group xuat hien lan dau."
        },

        // ==================================================================
        // TIMER / SFX / KEYFRAME ANIM / UI - object ID DA XAC MINH qua
        // objects.csv (Object-Popup-API). Field da verify truoc do qua header
        // that (TimerTriggerGameObject, SFXTriggerGameObject,
        // KeyframeAnimTriggerObject).
        // ==================================================================
        {
            "timer_trigger", "Timer Trigger", 3614,
            false, true, false, false, false, false,
            {
                { "targetItemId",     ParamType::Int, 0.f, 9999.f, 0.f },
                { "startTime",        ParamType::Float, 0.f, 999999.f, 0.f },   // m_startTime (double trong goc, thu gon Float)
                { "targetTime",       ParamType::Float, 0.f, 999999.f, 0.f },   // m_targetTime
                { "stopTimeEnabled",  ParamType::Bool, 0.f, 1.f, 0.f },
                { "dontOverride",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "ignoreTimeWarp",   ParamType::Bool, 0.f, 1.f, 0.f },
                { "timeMod",          ParamType::Float, 0.1f, 10.f, 1.f },
                { "startPaused",      ParamType::Bool, 0.f, 1.f, 0.f },
                { "multiActivate",    ParamType::Bool, 0.f, 1.f, 0.f },
                { "controlType",      ParamType::Int, 0.f, 3.f, 0.f },
            },
            { "timer", "bo dem thoi gian", "tao timer" },
            "Timer Trigger THAT - tao/khoi dong mot Timer Item moi (khac voi time_control_trigger/"
            "time_event_trigger chi doc/dieu khien timer co san). Object ID xac minh qua objects.csv."
        },
        {
            "sfx_trigger", "SFX Trigger", 3602,
            false, true, false, false, false, false,
            {
                { "soundID",       ParamType::Int, 0.f, 99999.f, 0.f },
                { "pitch",         ParamType::Float, 0.f, 4.f, 1.f },
                { "speed",         ParamType::Int, 0.f, 400.f, 100.f },
                { "volume",        ParamType::Float, 0.f, 2.f, 1.f },
                { "start",         ParamType::Int, 0.f, 999999.f, 0.f },
                { "fadeIn",        ParamType::Int, 0.f, 9999.f, 0.f },
                { "end",           ParamType::Int, 0.f, 999999.f, 0.f },
                { "fadeOut",       ParamType::Int, 0.f, 9999.f, 0.f },
                { "reverb",        ParamType::Bool, 0.f, 1.f, 0.f },
                { "loop",          ParamType::Bool, 0.f, 1.f, 0.f },
                { "unique",        ParamType::Bool, 0.f, 1.f, 0.f },
                { "sfxGroup",      ParamType::Int, 0.f, 9999.f, 0.f },
                { "stop",          ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "sfx", "am thanh", "sound effect" },
            "Phat mot file am thanh (SFX) da upload trong thu vien am thanh cua level. Field day du hon "
            "trong SFXTriggerGameObject (~35 field: volumeNear/Medium/Far, proximityMode...) chua map het, "
            "chi lay phan co ban hay dung nhat. DA XAC NHAN qua SetupReverbPopup::create(SFXTriggerGameObject*...) "
            "- 'reverb' KHONG PHAI mot trigger rieng ma la 1 THUOC TINH/POPUP CON cua chinh SFX Trigger nay "
            "(nguoi choi bam nut Reverb trong popup SFX de mo popup con chinh reverb preset/label). Field "
            "'reverb' bool o tren la dung, chi thieu chi tiet reverbPreset (chua map)."
        },
        {
            "edit_sfx_trigger", "Edit SFX Trigger", 3603,
            false, true, false, false, false, false,
            {
                { "sfxGroup",   ParamType::Int, 0.f, 9999.f, 0.f },
                { "volume",     ParamType::Float, 0.f, 2.f, 1.f },
                { "pitch",      ParamType::Float, 0.f, 4.f, 1.f },
                { "speed",      ParamType::Int, 0.f, 400.f, 100.f },
                { "stop",       ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "edit sfx", "chinh am thanh" },
            "Chinh volume/pitch/speed cua mot SFX dang phat (theo sfxGroup) hoac dung phat."
        },
        {
            "keyframe_animation_trigger", "Keyframe Animation Trigger", 3033,
            false, true, false, false, false, false,
            {
                { "targetGroup",  ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "timeMod",      ParamType::Float, 0.1f, 10.f, 1.f },
                { "positionXMod", ParamType::Float, -100.f, 100.f, 1.f },
                { "positionYMod", ParamType::Float, -100.f, 100.f, 1.f },
                { "rotationMod",  ParamType::Float, -100.f, 100.f, 1.f },
                { "scaleXMod",    ParamType::Float, -100.f, 100.f, 1.f },
                { "scaleYMod",    ParamType::Float, -100.f, 100.f, 1.f },
            },
            { "keyframe", "hoat hinh keyframe" },
            "Chay hoat hinh theo cac diem Keyframe da dat san (Keyframe Point), dieu chinh toc do/bien do "
            "qua cac he so mod. Chi co 1 loai duy nhat (khong tach Keyframe vs Keyframe Anim nhu nghi ngo truoc)."
        },

        // ---- Cac trigger moi khac (chua map field, chi co ID + mo ta) ------
        {
            "ui_trigger", "UI Trigger", 3613,
            false, true, false, false, false, false,
            {
                { "xRef",       ParamType::Int,  0.f, 3.f, 0.f },   // m_xRef - diem tham chieu ngang (vd: trai/giua/phai man hinh)
                { "yRef",       ParamType::Int,  0.f, 3.f, 0.f },   // m_yRef - diem tham chieu doc (vd: tren/giua/duoi man hinh)
                { "xRelative",  ParamType::Bool, 0.f, 1.f, 0.f },   // m_xRelative - toa do X tinh tuong doi theo % man hinh thay vi pixel
                { "yRelative",  ParamType::Bool, 0.f, 1.f, 0.f },   // m_yRelative
            },
            { "ui", "giao dien", "ui settings" },
            "DA VERIFY qua UISettingsGameObject that (ke thua EffectGameObject). Ten class that la "
            "'UISettingsGameObject' chu khong phai object tao UI moi - day la trigger THIET LAP CACH "
            "NEO (anchor) cho cac object UI dat trong level: xRef/yRef chon diem tham chieu tren man "
            "hinh (kieu 0-3: trai/giua/phai hoac tren/giua/duoi), xRelative/yRelative quyet dinh toa do "
            "cua object tinh theo % kich thuoc man hinh (relative/responsive) hay theo pixel co dinh. "
            "Field rat gon (chi 4) vi phan lon logic UI thuc te nam o cac object rieng le, trigger nay "
            "chi thiet lap tham so neo chung."
        },
        // ==================================================================
        // SHADER TRIGGERS - DA VERIFY qua ShaderGameObject that (ke thua
        // EffectGameObject). CA 17+ loai shader (bulge, pinch, hue, invert,
        // grayscale, sepia, chromatic, pixelate, lens circle, radial blur,
        // motion blur, glitch, chromatic glitch, noise, fisheye, shockwave,
        // shockline...) DUNG CHUNG 1 CLASS DUY NHAT voi cung 1 bo field -
        // moi loai chi khac nhau o object ID va lop UI chi hien field lien
        // quan (bulge/pinch dung targetX/targetY/inner/outer/maxSize; hue
        // dung m_speed lam hueShift; invert dung m_invert bool...). Field
        // duoi day la BO FIELD DAY DU CHUNG cho ca nhom, dung lam template.
        // Object ID cua tung loai con van CHUA xac dinh rieng le (tru
        // invert_color_trigger co ID cong dong 2921 - GIU NGUYEN, gan field
        // that vao).
        // ==================================================================
        {
            "invert_color_trigger", "Invert Color Trigger", 2921,
            false, true, false, false, false, false,
            {
                { "invert",        ParamType::Bool,  0.f, 1.f, 1.f },   // m_invert
                { "duration",      ParamType::Float, 0.f, 20.f, 0.5f }, // m_duration (ke thua EffectGameObject)
                { "fadeIn",        ParamType::Float, 0.f, 10.f, 0.f },  // m_fadeIn
                { "fadeOut",       ParamType::Float, 0.f, 10.f, 0.f },  // m_fadeOut
                { "zLayerMin",     ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",     ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",    ParamType::Bool,  0.f, 1.f, 0.f },   // m_disableAll: tat toan bo shader nay
                { "editorDisabled",ParamType::Bool,  0.f, 1.f, 0.f },
            },
            { "invert color", "dao mau" },
            "DA VERIFY qua ShaderGameObject that. Dao nguoc mau sac man hinh (hoac 1 vung theo zLayer) "
            "trong khoang thoi gian fadeIn/duration/fadeOut. La 1 trong 17+ loai Shader Trigger dung "
            "chung class ShaderGameObject - object ID rieng (2921) da xac nhan qua objects.csv."
        },
        // ------------------------------------------------------------------
        // 17 loai Shader Trigger cu the - OBJECT ID DA XAC MINH 100% qua
        // objects.csv (Object-Popup-API, dai 2904-2924, co 3 ID trong dai bi
        // bo trong - 2906/2908/2918 - khong ton tai trong game). Field DA
        // VERIFY qua ShaderGameObject that (26 field chung, ke thua tu
        // EffectGameObject) nhung MOI LOAI CHI DUNG MOT TAP CON field - tap
        // con cu the cho tung loai CHUA duoc xac nhan rieng (can save-string
        // mau tu editor de biet chinh xac editor hien field nao cho loai
        // nao). O day liet ke "best-guess" tap con hop ly nhat theo ten +
        // y nghia field, danh dau ro whichFieldsGuessed trong comment.
        // CAP NHAT: da bo sung 4 field chung zLayerMin/zLayerMax/disableAll/
        // editorDisabled cho ca 16 loai duoi day - day KHONG phai doan, ma la
        // field da duoc AIGenerate.cpp gan gia tri tu truoc (qua khoi
        // isShaderTrigger()) nhung schema truoc do quen khai bao.
        // ------------------------------------------------------------------
        {
            "shader_trigger", "Shader Trigger", 2904,
            false, true, false, false, false, false,
            {
                { "strength",      ParamType::Float, 0.f, 20.f, 1.f },
                { "fadeIn",        ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",       ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",     ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",     ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",    ParamType::Bool,  0.f, 1.f, 0.f },
                { "editorDisabled",ParamType::Bool,  0.f, 1.f, 0.f },
            },
            { "shader", "hieu ung man hinh goc" },
            "ID xac minh qua objects.csv (2904). Day la loai Shader Trigger CO BAN/GOC - kha nang la "
            "trigger 'cha' bat/tat toan bo he thong shader hoac 1 hieu ung don gian chua ro. Field cu "
            "the [DOAN, chua verify rieng] tu bo chung ShaderGameObject."
        },
        {
            "shock_wave_shader_trigger", "Shock Wave Shader Trigger", 2905,
            false, true, false, false, false, false,
            {
                { "targetX",   ParamType::Float, -30000.f, 30000.f, 0.f },
                { "targetY",   ParamType::Float, -30000.f, 30000.f, 0.f },
                { "strength",  ParamType::Float, 0.f, 20.f, 1.f },
                { "speed",     ParamType::Float, 0.f, 20.f, 1.f },
                { "maxSize",   ParamType::Float, 0.f, 1000.f, 500.f },
                { "fadeIn",    ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",   ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "shock wave", "song xung kich" },
            "ID xac minh qua objects.csv (2905). Field [DOAN theo ten - chua verify rieng tung field cho "
            "loai nay] tu bo chung ShaderGameObject: tam hieu ung (targetX/Y), do manh, toc do lan truyen, "
            "kich thuoc toi da. Da bo sung zLayerMin/Max, disableAll, editorDisabled (field chung, chac chan)."
        },
        {
            "shock_line_shader_trigger", "Shock Line Shader Trigger", 2907,
            false, true, false, false, false, false,
            {
                { "targetX",    ParamType::Float, -30000.f, 30000.f, 0.f },
                { "targetY",    ParamType::Float, -30000.f, 30000.f, 0.f },
                { "strength",   ParamType::Float, 0.f, 20.f, 1.f },
                { "speed",      ParamType::Float, 0.f, 20.f, 1.f },
                { "waveWidth",  ParamType::Float, 0.f, 100.f, 10.f },
                { "rotate",     ParamType::Bool,  0.f, 1.f, 0.f },
                { "fadeIn",     ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",    ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "shock line", "song xung kich thang" },
            "ID xac minh qua objects.csv (2907). Field [DOAN theo ten] giong Shock Wave nhung dang duong "
            "thang (waveWidth = do rong dai song, rotate = xoay huong dai song). Da bo sung zLayerMin/Max, "
            "disableAll, editorDisabled (field chung, chac chan)."
        },
        {
            "glitch_shader_trigger", "Glitch Shader Trigger", 2909,
            false, true, false, false, false, false,
            {
                { "strength", ParamType::Float, 0.f, 20.f, 1.f },
                { "speed",    ParamType::Float, 0.f, 20.f, 1.f },
                { "fadeIn",   ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",  ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "glitch", "nhieu hinh anh" },
            "ID xac minh qua objects.csv (2909). Field [DOAN theo ten] - do manh va toc do nhieu hinh. Da "
            "bo sung zLayerMin/Max, disableAll, editorDisabled (field chung, chac chan)."
        },
        {
            "chromatic_aberration_shader_trigger", "Chromatic Aberration Shader Trigger", 2910,
            false, true, false, false, false, false,
            {
                { "strength",      ParamType::Float, 0.f, 20.f, 1.f },
                { "screenOffsetX", ParamType::Float, -100.f, 100.f, 0.f },
                { "screenOffsetY", ParamType::Float, -100.f, 100.f, 0.f },
                { "fadeIn",        ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",       ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "chromatic aberration", "vien mau" },
            "ID xac minh qua objects.csv (2910). Field [DOAN theo ten] - do manh tach mau RGB va offset "
            "man hinh cho tung kenh mau. Da bo sung zLayerMin/Max, disableAll, editorDisabled (field chung, "
            "chac chan)."
        },
        {
            "chromatic_glitch_shader_trigger", "Chromatic Glitch Shader Trigger", 2911,
            false, true, false, false, false, false,
            {
                { "strength",      ParamType::Float, 0.f, 20.f, 1.f },
                { "speed",         ParamType::Float, 0.f, 20.f, 1.f },
                { "screenOffsetX", ParamType::Float, -100.f, 100.f, 0.f },
                { "screenOffsetY", ParamType::Float, -100.f, 100.f, 0.f },
                { "fadeIn",        ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",       ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "chromatic glitch", "nhieu vien mau" },
            "ID xac minh qua objects.csv (2911). Ket hop Glitch + Chromatic Aberration [DOAN theo ten]. Da "
            "bo sung zLayerMin/Max, disableAll, editorDisabled (field chung, chac chan)."
        },
        {
            "pixelate_shader_trigger", "Pixelate Shader Trigger", 2912,
            false, true, false, false, false, false,
            {
                { "strength", ParamType::Float, 1.f, 100.f, 4.f }, // kich thuoc pixel
                { "fadeIn",   ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",  ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "pixelate", "anh vuong pixel" },
            "ID xac minh qua objects.csv (2912). Field [DOAN theo ten] - strength dieu khien kich thuoc "
            "pixel. Da bo sung zLayerMin/Max, disableAll, editorDisabled (field chung, chac chan)."
        },
        {
            "lens_circle_shader_trigger", "Lens Circle Shader Trigger", 2913,
            false, true, false, false, false, false,
            {
                { "targetX", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "targetY", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "inner",   ParamType::Float, 0.f, 1000.f, 0.f },
                { "outer",   ParamType::Float, 0.f, 1000.f, 200.f },
                { "strength",ParamType::Float, 0.f, 20.f, 1.f },
                { "fadeIn",  ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut", ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "lens circle", "kinh lup tron" },
            "ID xac minh qua objects.csv (2913). Field [DOAN theo ten] - vung tron phong dai (inner/outer "
            "ban kinh) quanh tam targetX/Y. Da bo sung zLayerMin/Max, disableAll, editorDisabled (field "
            "chung, chac chan)."
        },
        {
            "radial_blur_shader_trigger", "Radial Blur Shader Trigger", 2914,
            false, true, false, false, false, false,
            {
                { "targetX", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "targetY", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "strength",ParamType::Float, 0.f, 20.f, 1.f },
                { "fadeIn",  ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut", ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "radial blur", "mo toa tron" },
            "ID xac minh qua objects.csv (2914). Field [DOAN theo ten] - lam mo toa ra tu 1 tam. Da bo sung "
            "zLayerMin/Max, disableAll, editorDisabled (field chung, chac chan)."
        },
        {
            "motion_blur_shader_trigger", "Motion Blur Shader Trigger", 2915,
            false, true, false, false, false, false,
            {
                { "strength", ParamType::Float, 0.f, 20.f, 1.f },
                { "useX",     ParamType::Bool,  0.f, 1.f, 1.f },
                { "useY",     ParamType::Bool,  0.f, 1.f, 0.f },
                { "fadeIn",   ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",  ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "motion blur", "mo chuyen dong" },
            "ID xac minh qua objects.csv (2915). Field [DOAN theo ten] - lam mo theo huong chuyen dong "
            "(useX/useY chon truc). Da bo sung zLayerMin/Max, disableAll, editorDisabled (field chung, "
            "chac chan)."
        },
        {
            "bulge_shader_trigger", "Bulge Shader Trigger", 2916,
            false, true, false, false, false, false,
            {
                { "targetX", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "targetY", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "inner",   ParamType::Float, 0.f, 1000.f, 0.f },
                { "outer",   ParamType::Float, 0.f, 1000.f, 200.f },
                { "maxSize", ParamType::Float, 0.f, 1000.f, 100.f },
                { "fadeIn",  ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut", ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "bulge", "phinh ra" },
            "ID xac minh qua objects.csv (2916). Field [DOAN theo ten, hop ly nhat vi ten field ThongTin "
            "khop voi hieu ung bulge/pinch] - tam targetX/Y, ban kinh trong/ngoai, do phinh toi da. Da bo "
            "sung zLayerMin/Max, disableAll, editorDisabled (field chung, chac chan)."
        },
        {
            "pinch_shader_trigger", "Pinch Shader Trigger", 2917,
            false, true, false, false, false, false,
            {
                { "targetX", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "targetY", ParamType::Float, -30000.f, 30000.f, 0.f },
                { "inner",   ParamType::Float, 0.f, 1000.f, 0.f },
                { "outer",   ParamType::Float, 0.f, 1000.f, 200.f },
                { "maxSize", ParamType::Float, 0.f, 1000.f, 100.f },
                { "fadeIn",  ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut", ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "pinch", "hut vao" },
            "ID xac minh qua objects.csv (2917). Cung cau truc field voi Bulge [DOAN theo ten] - chi khac "
            "chieu hieu ung (hut vao thay vi phinh ra). Da bo sung zLayerMin/Max, disableAll, editorDisabled "
            "(field chung, chac chan)."
        },
        {
            "grayscale_shader_trigger", "Grayscale Shader Trigger", 2919,
            false, true, false, false, false, false,
            {
                { "strength", ParamType::Float, 0.f, 1.f, 1.f },
                { "fadeIn",   ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",  ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "grayscale", "den trang", "khu mau" },
            "ID xac minh qua objects.csv (2919). Field [DOAN theo ten] - strength = muc do khu mau (0=giu "
            "nguyen mau, 1=den trang hoan toan). Da bo sung zLayerMin/Max, disableAll, editorDisabled "
            "(field chung, chac chan)."
        },
        {
            "sepia_shader_trigger", "Sepia Shader Trigger", 2920,
            false, true, false, false, false, false,
            {
                { "strength", ParamType::Float, 0.f, 1.f, 1.f },
                { "fadeIn",   ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",  ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "sepia", "mau nau co dien" },
            "ID xac minh qua objects.csv (2920). Field [DOAN theo ten] - tuong tu Grayscale nhung tong mau "
            "nau (sepia) thay vi den trang. Da bo sung zLayerMin/Max, disableAll, editorDisabled (field "
            "chung, chac chan)."
        },
        {
            "hue_shader_trigger", "Hue Shader Trigger", 2922,
            false, true, false, false, false, false,
            {
                { "speed",    ParamType::Float, -360.f, 360.f, 0.f }, // hueShift
                { "strength", ParamType::Float, 0.f, 1.f, 1.f },
                { "fadeIn",   ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",  ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "hue", "doi tong mau", "hue shift" },
            "ID xac minh qua objects.csv (2922). Field [DOAN theo ten] - speed dung lam do lech hue "
            "(hueShift, tinh theo do), strength = cuong do ap dung. Da bo sung zLayerMin/Max, disableAll, "
            "editorDisabled (field chung, chac chan)."
        },
        {
            "edit_color_shader_trigger", "Color Change Shader Trigger", 2923,
            false, true, false, false, false, false,
            {
                { "colorR",   ParamType::Int,   0.f, 255.f, 255.f },
                { "colorG",   ParamType::Int,   0.f, 255.f, 255.f },
                { "colorB",   ParamType::Int,   0.f, 255.f, 255.f },
                { "strength", ParamType::Float, 0.f, 1.f, 1.f },
                { "fadeIn",   ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut",  ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "color change", "doi mau shader", "tint man hinh" },
            "ID xac minh qua objects.csv (2923). DA XAC NHAN qua SetupShaderEffectPopup::setupColorChange() "
            "- day la shader PHU 1 LOP MAU LEN TOAN MAN HINH (tint/overlay), KHONG PHAI co che 'chinh sua "
            "shader dang chay' nhu doan truoc. Doi ten hien thi tu 'Edit Color' sang 'Color Change' cho "
            "dung ban chat. Field RGB+strength+fadeIn/Out [DOAN theo cau truc tuong tu color_trigger, "
            "chua verify tung field rieng qua save-string that]. Da bo sung zLayerMin/Max, disableAll, "
            "editorDisabled (field chung, chac chan)."
        },
        {
            "split_screen_shader_trigger", "Split Screen Shader Trigger", 2924,
            false, true, false, false, false, false,
            {
                { "useX",    ParamType::Bool,  0.f, 1.f, 1.f },
                { "useY",    ParamType::Bool,  0.f, 1.f, 0.f },
                { "fadeIn",  ParamType::Float, 0.f, 10.f, 0.f },
                { "fadeOut", ParamType::Float, 0.f, 10.f, 0.f },
                { "zLayerMin",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "zLayerMax",      ParamType::Int,  -999.f, 999.f, 0.f },
                { "disableAll",     ParamType::Bool, 0.f, 1.f, 0.f },
                { "editorDisabled", ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "split screen", "chia man hinh" },
            "ID xac minh qua objects.csv (2924). [CHUA RO Y NGHIA CHINH XAC, do tin cay THAP] - co the "
            "lien quan Two Player mode (chia man hinh cho 2 nguoi choi) chu khong phai hieu ung shader "
            "thong thuong. Field o day chi la doan tam. Da bo sung zLayerMin/Max, disableAll, "
            "editorDisabled (field chung, chac chan)."
        },
        {
            "keyframe_point", "Keyframe Point", 3032,
            false, false, true, false, false, false,
            {
                { "keyframeGroup", ParamType::GroupId, 0.f, 9999.f, 0.f }, // m_keyframeGroup - group ID cua diem nay
                { "keyframeIndex", ParamType::Int, 0.f, 999.f, 0.f },      // m_keyframeIndex - thu tu diem trong chuoi
                { "referenceOnly", ParamType::Bool, 0.f, 1.f, 0.f },       // m_referenceOnly - chi dung de tham chieu, khong render
                { "proximity",     ParamType::Bool, 0.f, 1.f, 0.f },       // m_proximity - kich hoat theo khoang cach thay vi thoi gian
                { "curve",         ParamType::Bool, 0.f, 1.f, 0.f },       // m_curve - noi diem nay bang duong cong thay vi thang
                { "closeLoop",     ParamType::Bool, 0.f, 1.f, 0.f },       // m_closeLoop - khep kin chuoi thanh vong lap
                { "timeMode",      ParamType::Int,  0.f, 2.f, 0.f },       // m_timeMode - che do tinh thoi gian den diem tiep theo
                { "spawnDelay",    ParamType::Float, 0.f, 60.f, 0.f },     // m_spawnDelay
                { "previewArt",    ParamType::Bool, 0.f, 1.f, 1.f },       // m_previewArt - hien preview duong di trong editor
                { "autoLayer",     ParamType::Bool, 0.f, 1.f, 0.f },       // m_autoLayer
                { "direction",     ParamType::Int,  0.f, 3.f, 0.f },       // m_direction
                { "revolutions",   ParamType::Int,  0.f, 20.f, 0.f },      // m_revolutions - so vong quay neu la duong cong tron
                { "lineOpacity",   ParamType::Float, 0.f, 1.f, 1.f },      // m_lineOpacity - do trong suot duong preview trong editor
            },
            { "keyframe point", "diem keyframe" },
            "DA VERIFY qua KeyframeGameObject that (ke thua EffectGameObject). Object DAT TRUC TIEP danh "
            "dau 1 diem trong khong gian, cac diem cung keyframeGroup duoc noi lai thanh 1 chuoi theo thu "
            "tu keyframeIndex de Keyframe Animation Trigger chay hoat hinh doc theo. curve=true de noi "
            "bang duong cong (dung spline, xem KeyframeObject runtime helper); closeLoop khep vong; "
            "proximity doi cach tinh thoi gian den diem ke tiep sang dua theo khoang cach; referenceOnly "
            "de dat 1 diem chi dung lam tham chieu (vd tam xoay) khong nam trong duong di chinh. Field "
            "m_unk760 (giua timeMode va spawnDelay) CHUA RO y nghia, bo qua."
        },
        {
            "old_end_trigger", "Old End Trigger", 1931,
            false, true, false, false, false, false,
            {},
            { "old end", "end cu" },
            "[CHUA MAP FIELD] End Trigger phien ban cu, khac voi end_trigger (3600) hien tai."
        },

        // ==================================================================
        // CHECKPOINT / FORCE BLOCK - DA VERIFY qua CheckpointGameObject va
        // ForceBlockGameObject that (ke thua EffectGameObject). Xac nhan day
        // KHONG phai trigger activate/deactivate thong thuong ma la OBJECT
        // DAT TRUC TIEP trong level (co the collide/tuong tac vat ly), field
        // rat gon vi phan lon logic nam trong runtime chu khong phai save data.
        // ==================================================================
        {
            "checkpoint_trigger", "Checkpoint", 2063,
            false, false, false, false, false, false,
            {
                { "respawnID", ParamType::Int, 0.f, 999.f, 0.f }, // m_respawnID - dung de xac dinh checkpoint nao gan nhat khi respawn
            },
            { "checkpoint", "diem hoi sinh", "luu diem" },
            "DA VERIFY qua CheckpointGameObject that (ke thua EffectGameObject, chi them "
            "m_checkpointActivated [runtime state, khong luu] va m_respawnID). Day la OBJECT DAT TRONG "
            "LEVEL (Practice Mode) chu khong phai trigger kich hoat tu group - khi nguoi choi cham qua se "
            "luu vi tri hoi sinh. respawnID dung khi co nhieu checkpoint can quan ly rieng."
        },
        {
            "force_block_trigger", "Force Block", 2069,
            false, false, false, false, false, false,
            {
                { "force",         ParamType::Float, -100.f, 100.f, 1.f },  // m_force
                { "minForce",      ParamType::Float, 0.f, 100.f, 0.f },     // m_minForce
                { "maxForce",      ParamType::Float, 0.f, 100.f, 10.f },    // m_maxForce
                { "relativeForce", ParamType::Bool,  0.f, 1.f, 0.f },       // m_relativeForce
                { "forceRange",    ParamType::Bool,  0.f, 1.f, 0.f },       // m_forceRange: bat che do min/max thay vi force co dinh
                { "forceID",       ParamType::Int,   0.f, 9999.f, 0.f },    // m_forceID
            },
            { "force block", "khoi day", "wind block" },
            "DA VERIFY qua ForceBlockGameObject that (ke thua EffectGameObject). LA OBJECT VAT LY dat "
            "truc tiep trong level (khong phai trigger kich hoat tu xa) - tao luc day/keo (nhu gio) len "
            "cac object/player di qua vung anh huong cua no. calculateForceToTarget() cho thay co tinh "
            "toan huong luc theo vi tri object muc tieu."
        },
        {
            "force_circle_trigger", "Force Circle", 3645,
            false, false, false, false, false, false,
            {
                { "force",         ParamType::Float, -100.f, 100.f, 1.f },
                { "minForce",      ParamType::Float, 0.f, 100.f, 0.f },
                { "maxForce",      ParamType::Float, 0.f, 100.f, 10.f },
                { "relativeForce", ParamType::Bool,  0.f, 1.f, 0.f },
                { "forceRange",    ParamType::Bool,  0.f, 1.f, 0.f },
                { "forceID",       ParamType::Int,   0.f, 9999.f, 0.f },
            },
            { "force circle", "vung day tron" },
            "[CHUA CO CLASS RIENG - dung tam field cua ForceBlockGameObject vi ten/hanh vi tuong tu, "
            "chi khac vung anh huong la hinh tron thay vi hinh chu nhat] Object vat ly tao luc day/keo "
            "dang tron. CAN VERIFY LAI qua ForceCircleGameObject that neu ton tai, hien dang dung chung "
            "field voi Force Block vi chua tim thay class rieng."
        },

        // ==================================================================
        // CAMERA TRIGGERS
        // ==================================================================
        {
            "camera_zoom_trigger", "Camera Zoom Trigger", 1913,
            true, false, false, false, false, false,
            {
                { "duration", ParamType::Float, 0.f, 60.f, 0.5f },
                { "zoom",     ParamType::Float, 0.1f, 20.f, 1.f },
            },
            { "camera zoom", "zoom camera" },
            "Thay doi do zoom cua camera trong mot khoang thoi gian."
        },
        {
            "camera_static_trigger", "Camera Static Trigger", 1914,
            false, true, false, false, false, false,
            {
                { "targetGroup",        ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "editCameraSettings", ParamType::Bool, 0.f, 1.f, 0.f },
                { "disableGridSnap",    ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "camera static", "khoa camera" },
            "Khoa camera vao vi tri co dinh thay vi bam theo nguoi choi."
        },
        {
            "camera_offset_trigger", "Camera Offset Trigger", 1916,
            true, false, false, false, false, false,
            {
                { "duration",     ParamType::Float, 0.f, 60.f, 0.5f },
                { "offsetX",      ParamType::Float, -1000.f, 1000.f, 0.f },
                { "offsetY",      ParamType::Float, -1000.f, 1000.f, 0.f },
                { "cameraEasing", ParamType::Float, 0.f, 10.f, 0.5f },
            },
            { "camera offset", "lech camera" },
            "Dich camera theo mot offset X/Y so voi vi tri mac dinh."
        },
        {
            "camera_gp_offset_trigger", "Camera GP Offset Trigger", 2901,
            true, false, false, false, false, false,
            {
                { "duration",     ParamType::Float, 0.f, 60.f, 0.5f },
                { "offsetX",      ParamType::Float, -1000.f, 1000.f, 0.f },
                { "offsetY",      ParamType::Float, -1000.f, 1000.f, 0.f },
                { "cameraEasing", ParamType::Float, 0.f, 10.f, 0.5f },
            },
            { "camera gp offset", "camera offset gameplay" },
            "Giong Camera Offset nhung ap dung theo he toa do gameplay thay vi editor."
        },
        {
            "camera_rotate_trigger", "Camera Rotate Trigger", 2015,
            true, false, false, false, true, false,
            {
                { "duration",     ParamType::Float, 0.f, 60.f, 0.5f },
                { "degrees",      ParamType::Float, -3600.f, 3600.f, 90.f },
                { "cameraEasing", ParamType::Float, 0.f, 10.f, 0.5f },
            },
            { "camera rotate", "xoay camera" },
            "Xoay goc nhin camera mot so do trong mot khoang thoi gian."
        },
        {
            "camera_edge_trigger", "Camera Edge Trigger", 2062,
            false, true, false, false, false, false,
            {
                { "edgeDirection",    ParamType::Int, 0.f, 4.f, 0.f },
                { "followObject",     ParamType::Bool, 0.f, 1.f, 0.f },   // m_followObject (da doi ten dung)
                { "followEasing",     ParamType::Float, 0.f, 10.f, 1.f },
                { "smoothVelocity",   ParamType::Bool, 0.f, 1.f, 0.f },
                { "velocityModifier", ParamType::Float, 0.f, 10.f, 0.f },
                { "exitInstant",      ParamType::Bool, 0.f, 1.f, 0.f },
                { "exitStatic",       ParamType::Bool, 0.f, 1.f, 0.f },
                { "previewOpacity",   ParamType::Float, 0.f, 1.f, 1.f },
            },
            { "camera edge", "gioi han camera" },
            "Gioi han pham vi di chuyen cua camera theo canh. DA XAC MINH 100% qua CameraTriggerGameObject "
            "that - du 8 field, khong thieu."
        },
        {
            "camera_mode_trigger", "Camera Mode Trigger", 2925,
            false, true, false, false, false, false,
            {},
            { "camera mode", "che do camera" },
            "Doi che do hoat dong cua camera (vd platformer camera mode)."
        },
        {
            "collision_trigger", "Collision Trigger", 1815,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "collision", "va cham" },
            "Kich hoat group khi 2 collision block da dinh cham vao nhau."
        },
        {
            "midground_setup_trigger", "Midground Setup Trigger", 2999,
            false, true, false, false, false, false,
            {},
            { "midground setup", "thiet lap lop giua" },
            "Thiet lap thong so ban dau cho lop middleground."
        },
        {
            "bg_speed_trigger", "BG Speed Trigger", 3606,
            true, false, false, false, false, false,
            { { "duration", ParamType::Float, 0.f, 60.f, 0.5f } },
            { "bg speed", "toc do nen" },
            "Thay doi toc do cuon cua background trong mot khoang thoi gian."
        },
        {
            "midground_speed_trigger", "Midground Speed Trigger", 3612,
            true, false, false, false, false, false,
            { { "duration", ParamType::Float, 0.f, 60.f, 0.5f } },
            { "midground speed", "toc do lop giua" },
            "Thay doi toc do cuon cua middleground trong mot khoang thoi gian."
        },
        {
            "instant_collision_trigger", "Instant Collision Trigger", 3609,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "instant collision" },
            "Kiem tra ngay lap tuc trang thai va cham giua 2 block va kich hoat group tuong ung."
        },
        {
            "on_death_trigger", "On Death Trigger", 1812,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "on death", "khi chet" },
            "Kich hoat mot group ngay khi nguoi choi chet."
        },
        {
            "player_control_trigger", "Player Control Trigger", 1932,
            false, true, false, false, false, false,
            {},
            { "player control", "dieu khien nguoi choi" },
            "Bat/tat quyen dieu khien cua nguoi choi."
        },

        // ==================================================================
        // COLOR / GRADIENT / GRAVITY / SHAKE / TIME WARP
        // ==================================================================
        {
            "color_trigger", "Color Trigger", 899,
            true, false, false, false, false, true,
            {
                { "duration", ParamType::Float, 0.f, 60.f, 0.5f },
                { "channel",  ParamType::Int,   0.f, 9999.f, 1000.f },
                { "colorR",   ParamType::Int,   0.f, 255.f, 255.f },
                { "colorG",   ParamType::Int,   0.f, 255.f, 255.f },
                { "colorB",   ParamType::Int,   0.f, 255.f, 255.f },
                { "opacity",  ParamType::Float, 0.f, 1.f, 1.f },
                { "blending", ParamType::Bool,  0.f, 1.f, 0.f },
            },
            { "color", "doi mau", "mau sac", "background color", "ground color" },
            "Doi mau cua mot Color Channel sang mau RGB chi dinh trong mot khoang thoi gian."
        },
        {
            "gradient_trigger", "Gradient Trigger", 2903,
            false, true, false, false, false, false,
            {
                { "targetGroup",  ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "vertexMode",   ParamType::Bool, 0.f, 1.f, 0.f },
                { "groupUp",      ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "groupDown",    ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "groupLeft",    ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "groupRight",   ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "blendingMode", ParamType::Int, 0.f, 1.f, 0.f },
                { "disable",      ParamType::Bool, 0.f, 1.f, 0.f },
            },
            { "gradient", "do mau chuyen tiep", "dai mau" },
            "Tao mot vung mau gradient phu len group, gan mau tu 4 group huong Up/Down/Left/Right."
        },
        {
            "shake_trigger", "Shake Trigger", 1520,
            true, false, true, false, false, false,
            {
                { "duration", ParamType::Float, 0.f, 60.f, 0.5f },
                { "strength", ParamType::Float, 0.f, 50.f, 5.f },
                { "interval", ParamType::Float, 0.f, 5.f, 0.03f },
            },
            { "shake", "rung man hinh", "camera shake" },
            "Lam rung man hinh/camera trong mot khoang thoi gian."
        },
        {
            "gravity_trigger", "Gravity Trigger", 2066,
            false, true, false, false, false, false,
            { { "gravityValue", ParamType::Float, -2.f, 2.f, 1.f } },
            { "gravity", "trong luc", "doi trong luc" },
            "Doi huong/do lon trong luc cua nguoi choi ngay lap tuc."
        },
        {
            "time_warp_trigger", "Time Warp Trigger", 1935,
            true, false, false, false, false, false,
            {
                { "duration",  ParamType::Float, 0.f, 60.f, 0.5f },
                { "timeMod",   ParamType::Float, 0.1f, 10.f, 1.f },
            },
            { "time warp", "chinh toc do thoi gian", "slowdown" },
            "Thay doi toc do troi cua thoi gian trong level."
        },

        // ==================================================================
        // ITEM EDIT / COMPARE
        // ==================================================================
        {
            "item_edit_trigger", "Item Edit Trigger", 3619,
            false, true, false, false, false, false,
            {
                { "targetItemId1",   ParamType::Int, 0.f, 9999.f, 0.f },      // ke thua tu EffectGameObject, ten chua verify 100%
                { "item1Mode",       ParamType::Int, 0.f, 2.f, 0.f },          // m_item1Mode
                { "targetItemId2",   ParamType::Int, 0.f, 9999.f, 0.f },
                { "item2Mode",       ParamType::Int, 0.f, 2.f, 0.f },          // m_item2Mode
                // KHONG co "resultItemId" - da kiem tra ItemTriggerGameObject.hpp
                // that: item edit trigger GHI DE ket qua LEN CHINH item1
                // (targetItemId1), khong co item ID thu 3 rieng cho ket qua.
                { "targetItemMode",  ParamType::Int, 0.f, 2.f, 0.f },          // m_targetItemMode - MOI, che do cho result item
                { "mod1",            ParamType::Float, -1000.f, 1000.f, 1.f }, // m_mod1 (doi ten tu modifier1)
                { "mod2",            ParamType::Float, -1000.f, 1000.f, 1.f }, // m_mod2
                { "signType1",       ParamType::Int, 0.f, 2.f, 0.f },          // m_signType1
                { "signType2",       ParamType::Int, 0.f, 2.f, 0.f },          // m_signType2
                { "resultType1",     ParamType::Int, 0.f, 3.f, 0.f },          // m_resultType1 - MOI, chua ro y nghia enum
                { "resultType2",     ParamType::Int, 0.f, 3.f, 0.f },          // m_resultType2 - MOI
                { "resultType3",     ParamType::Int, 0.f, 3.f, 0.f },          // m_resultType3 - MOI
                { "roundType1",      ParamType::Int, 0.f, 2.f, 0.f },          // m_roundType1
                { "roundType2",      ParamType::Int, 0.f, 2.f, 0.f },          // m_roundType2
                { "tolerance",       ParamType::Float, 0.f, 100.f, 0.f },      // m_tolerance
                { "persistent",      ParamType::Bool, 0.f, 1.f, 0.f },         // m_persistent
                { "targetAll",       ParamType::Bool, 0.f, 1.f, 0.f },         // m_targetAll
                { "reset",           ParamType::Bool, 0.f, 1.f, 0.f },         // m_reset (doi ten tu resetToggle)
                { "useTimer",        ParamType::Bool, 0.f, 1.f, 0.f },         // m_timer (doi ten tu useTimer, giu nguyen ten hien thi)
            },
            { "item edit", "tinh toan item", "gan gia tri item" },
            "DA VERIFY qua ItemTriggerGameObject that. Thuc hien phep toan (mod1/mod2 + signType, lam tron qua "
            "roundType) giua targetItemId1/2 va ghi vao resultItemId (theo targetItemMode). resultType1/2/3 la "
            "3 field MOI phat hien, y nghia enum cu the CHUA XAC DINH (co the lien quan kieu du lieu tra ve: "
            "Item/Timer/Points...). ID cua targetItemId1/2/resultItemId ke thua tu EffectGameObject nen ten "
            "field chua verify duoc 100% qua file nay."
        },
        {
            "item_compare_trigger", "Item Compare Trigger", 3620,
            false, true, false, false, false, false,
            {
                { "targetGroup",    ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "item1",          ParamType::Int, 0.f, 9999.f, 0.f },
                { "item2",          ParamType::Int, 0.f, 9999.f, 0.f },
                { "comparisonType", ParamType::Int, 0.f, 2.f, 1.f },
                { "tolerance",      ParamType::Float, 0.f, 100.f, 0.f },
            },
            { "item compare", "so sanh item", "if item" },
            "So sanh gia tri giua 2 Item ID va kich hoat group tuong ung neu dung."
        },
        {
            "reverse_trigger", "Reverse Trigger", 1917,
            false, true, false, false, false, false,
            {},
            { "reverse", "dao nguoc huong" },
            "Dao nguoc huong di chuyen cua nguoi choi ngay lap tuc."
        },
        {
            "end_trigger", "End Trigger", 3600,
            false, true, false, false, false, false,
            {},
            { "end", "ket thuc level", "finish" },
            "Ket thuc level ngay lap tuc tai vi tri trigger duoc kich hoat."
        },
        {
            "time_control_trigger", "Time Control Trigger", 3617,
            false, true, false, false, false, false,
            { { "targetItemId", ParamType::Int, 0.f, 9999.f, 0.f } },
            { "time control", "dieu khien timer", "bat/dung timer" },
            "Bat, dung, hoac dat lai mot Timer Item da co san."
        },
        {
            "time_event_trigger", "Time Event Trigger", 3615,
            false, true, false, false, false, false,
            {
                { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f },
                { "targetItemId", ParamType::Int, 0.f, 9999.f, 0.f },
            },
            { "time event", "kich hoat theo timer" },
            "Kich hoat mot group khi gia tri cua mot Timer Item dat moc thoi gian chi dinh."
        },
        {
            "event_trigger", "Event Trigger", 3604,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "event", "su kien" },
            "Kich hoat group khi mot su kien gameplay cu the xay ra."
        },
        {
            "song_trigger", "Song Trigger", 1934,
            false, true, false, false, false, false,
            {
                { "duration",     ParamType::Float, 0.f, 60.f, 0.5f },
                { "songChannel",  ParamType::Int, 0.f, 4.f, 0.f },
            },
            { "song", "phat nhac", "play song" },
            "Phat mot kenh nhac da thiet lap san."
        },
        {
            "edit_song_trigger", "Edit Song Trigger", 3605,
            false, true, false, false, false, false,
            { { "duration", ParamType::Float, 0.f, 60.f, 0.5f } },
            { "edit song", "chinh nhac", "fade nhac", "volume nhac" },
            "Chinh am luong/pitch/tocdo cua mot kenh nhac dang phat."
        },
        {
            "options_trigger", "Options Trigger", 2899,
            false, true, false, false, false, false,
            {},
            { "options", "tuy chon he thong", "two player", "no fx" },
            "Bat/tat cac tuy chon he thong cua level."
        },
        {
            "persistent_item_setup_trigger", "Persistent Item Setup Trigger", 3641,
            false, true, false, false, false, false,
            { { "targetItemId", ParamType::Int, 0.f, 9999.f, 0.f } },
            { "persistent item", "luu item xuyen level", "item pers" },
            "Thiet lap mot Item ID la 'persistent' - giu nguyen xuyen suot cac lan choi."
        },
        {
            "hide_player_trigger", "Hide Player Trigger", 1612,
            false, true, false, false, false, false,
            {}, { "hide player", "an nguoi choi" },
            "An icon nguoi choi ngay lap tuc."
        },
        {
            "show_player_trigger", "Show Player Trigger", 1613,
            false, true, false, false, false, false,
            {}, { "show player", "hien nguoi choi" },
            "Hien lai icon nguoi choi ngay lap tuc."
        },
        {
            "background_effect_on_trigger", "Background Effect On Trigger", 1818,
            false, true, false, false, false, false,
            {}, { "background effect on", "bat hieu ung nen" },
            "Bat hieu ung an nut background/nen ngay lap tuc."
        },
        {
            "background_effect_off_trigger", "Background Effect Off Trigger", 1819,
            false, true, false, false, false, false,
            {}, { "background effect off", "tat hieu ung nen" },
            "Tat hieu ung an nut background/nen ngay lap tuc."
        },
        {
            "no_enter_effect_trigger", "No Enter Effect Trigger", 1915,
            false, true, false, false, false, false,
            {}, { "no enter effect", "tat enter effect" },
            "Tat hieu ung Enter Effect cho cac object sau do."
        },
        {
            "gameplay_rotation_trigger", "Gameplay Rotation Trigger", 2900,
            true, false, false, false, true, false,
            {
                { "duration", ParamType::Float, 0.f, 60.f, 0.5f },
                { "degrees",  ParamType::Float, -3600.f, 3600.f, 90.f },
            },
            { "gameplay rotation", "xoay gameplay" },
            "Xoay he toa do gameplay trong mot khoang thoi gian."
        },
        {
            "teleport_trigger", "Teleport Trigger", 3022,
            false, true, true, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "teleport", "dich chuyen tuc thoi" },
            "Dich chuyen nguoi choi ngay lap tuc den vi tri cua mot group dich."
        },
        {
            "enter_stop_trigger", "Enter Stop Trigger", 3023,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "enter stop", "dung enter effect" },
            "Dung ngay lap tuc hieu ung Enter dang chay tren mot group."
        },
        {
            "area_stop_trigger", "Area Stop Trigger", 3024,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "area stop", "dung area effect" },
            "Dung ngay lap tuc hieu ung Area dang chay tren mot group."
        },
        {
            "object_control_trigger", "Object Control Trigger", 3655,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "object control", "dieu khien object" },
            "Dieu khien cac thuoc tinh dac biet cua mot group ngay lap tuc."
        },
        {
            "link_visible_trigger", "Link Visible Trigger", 3662,
            false, true, false, false, false, false,
            { { "targetGroup", ParamType::GroupId, 0.f, 9999.f, 0.f } },
            { "link visible", "lien ket hien thi" },
            "Lien ket trang thai an/hien giua cac group duoc lien ket."
        },
        {
            "bpm_trigger", "BPM Trigger", 3642,
            false, true, false, false, false, false,
            {},
            { "bpm", "nhip do", "beat per minute" },
            "Thiet lap gia tri BPM tu diem nay tro di."
        },
        };
        // 3 tham so CHUNG cho MOI trigger (ke thua tu EffectGameObject that:
        // m_isTouchTriggered, m_isSpawnTriggered, m_isMultiTriggered - da
        // verify qua header binding that, khong phai doan). Gan chung o day
        // thay vi liet ke lai trong tung entry o tren cho do trung lap.
        const std::vector<TriggerParam> kUniversalParams = {
            { "touchTriggered",  ParamType::Bool, 0.f, 1.f, 0.f },
            { "spawnTriggered",  ParamType::Bool, 0.f, 1.f, 0.f },
            { "multiTriggered",  ParamType::Bool, 0.f, 1.f, 0.f },
        };

        std::vector<TriggerSchema> result;
        result.reserve(temp.size());
        for (const auto& entry : temp) {
            auto params = entry.params;
            params.insert(params.end(), kUniversalParams.begin(), kUniversalParams.end());
            result.push_back(TriggerSchema{
                entry.gdObjectId,
                entry.id,
                entry.displayName,
                entry.description,
                entry.aliases,
                params
            });
        }
        return result;
    }();
    return table;
}

const ta::TriggerSchema* findSchema(const std::string& id) {
    for (const auto& s : triggerSchemas()) { // <-- Thêm () vào đây
        if (s.id == id) return &s;
    }
    return nullptr;
}

} // namespace ta

// ============================================================================
// TRIGGER DA CO FIELD DAY DU (xac minh qua header that) NHUNG VAN CHUA CO
// OBJECT ID THAT - san sang them ngay khi xac dinh duoc ID:
//
//  - Timer Trigger that (TimerTriggerGameObject): startTime, targetTime,
//    stopTimeEnabled, dontOverride, ignoreTimeWarp, timeMod, startPaused,
//    multiActivate, controlType.
//  - Event Link Trigger (EventLinkTrigger): eventIDs (set<int>), resetRemap,
//    extraID, extraID2.
//  - SFX Trigger (SFXTriggerGameObject): ~35 field (soundID, pitch, speed,
//    volume, start/fadeIn/end/fadeOut, reverb+reverbPreset, loop, unique,
//    override, volumeNear/Medium/Far, proximityMode, sfxGroup, variance...).
//  - Keyframe Anim Trigger (KeyframeAnimTriggerObject): timeMod,
//    positionXMod/YMod, rotationMod, scaleXMod/YMod.
//
// NGHI VAN CAN GIAI QUYET:
//  - item_edit_trigger: van dang suy doan property ID, can file field-
//    declaration that cua ItemTriggerGameObject (file .hpp gui truoc do
//    chi la modify-hook, khong co field list).
//  - Force Circle (3645): dang TAM dung chung field voi Force Block do ten/
//    hanh vi tuong tu - CAN VERIFY LAI qua class that (ForceCircleGameObject?)
//    neu tim duoc, hien khong ro co phai ke thua ForceBlockGameObject khong.
//  - 17/18 loai Shader Trigger (tru Invert Color): field cu the (targetX/Y,
//    inner/outer, waveWidth...) VAN la DOAN theo ten (du da xac nhan SO
//    LUONG va TEN HAM setup qua SetupShaderEffectPopup). Rieng 4 field chung
//    zLayerMin/Max, disableAll, editorDisabled thi DA CHAC CHAN (suy ra tu
//    AIGenerate.cpp), khong con la doan nua. CAN save-string mau tu editor
//    GD de xac nhan chinh xac ten/gia tri cac field con lai cho tung loai.
// ============================================================================