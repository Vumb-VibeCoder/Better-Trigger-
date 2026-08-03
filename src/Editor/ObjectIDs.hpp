#pragma once

#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

// ============================================================================
// Dong bo voi TriggerSchema.cpp. Cap nhat lan nay: xac minh + bo sung ID that
// tu Object-Popup-API (resources/objects.csv - bang ID chinh thuc toan bo game).
// Them: TIME_TRIGGER (Timer that), SFX/EDIT_SFX, KEYFRAME_ANIMATION,
// UI_TRIGGER, CHECKPOINT, FORCE_BLOCK/CIRCLE, SHADER_TRIGGER + 17 loai con,
// ENTER_MOVE/ROTATE/SCALE/FADE/TINT (NHOM MOI, khac Area Trigger),
// OLD_END_TRIGGER, LEVEL_END_TRIGGER_1_0, INVERT_COLOR_TRIGGER.
// Sua ten: MIDGROUND_SETUP_TRIGGER -> EDIT_MIDDLEGROUND_TRIGGER (ten dung theo
// objects.csv, ID giu nguyen 2999).
//
// CAP NHAT MOI: them ham helper isShaderTrigger(int) - dung cho AIGenerate.cpp
// de nhan dien ca 17 loai Shader Trigger bang 1 dieu kien duy nhat thay vi
// liet ke tung ID rieng le. Dai ID shader: 2904-2924, tru 3 slot bo trong
// khong dung (2906, 2908, 2918).
// ============================================================================

namespace object_ids {
    // ---- Move / Rotate / Scale / Alpha ----
    static constexpr int MOVE_TRIGGER = 901;
    static constexpr int ROTATE_TRIGGER = 1346;
    static constexpr int SCALE_TRIGGER = 2067;
    static constexpr int ALPHA_TRIGGER = 1007;
    static constexpr int TOGGLE_TRIGGER = 1049;
    static constexpr int SPAWN_TRIGGER = 1268;

    // ---- Advanced Follow ----
    static constexpr int ADVANCED_FOLLOW_TRIGGER = 3016;
    static constexpr int PULSE_TRIGGER = 1006;
    static constexpr int STOP_TRIGGER = 1616;
    static constexpr int FOLLOW_TRIGGER = 1347;
    static constexpr int ANIMATE_TRIGGER = 1585;
    static constexpr int FOLLOW_PLAYER_Y_TRIGGER = 1814;
    static constexpr int EDIT_ADVANCED_FOLLOW_TRIGGER = 3660;
    static constexpr int RE_TARGET_ADVANCED_FOLLOW_TRIGGER = 3661;

    // ---- Area Triggers (lan truyen theo nhieu object trong vung) ----
    static constexpr int AREA_MOVE_TRIGGER = 3006;
    static constexpr int AREA_ROTATE_TRIGGER = 3007;
    static constexpr int AREA_SCALE_TRIGGER = 3008;
    static constexpr int AREA_FADE_TRIGGER = 3009;
    static constexpr int AREA_TINT_TRIGGER = 3010;
    static constexpr int EDIT_AREA_MOVE_TRIGGER = 3011;
    static constexpr int EDIT_AREA_ROTATE_TRIGGER = 3012;
    static constexpr int EDIT_AREA_SCALE_TRIGGER = 3013;
    static constexpr int EDIT_AREA_FADE_TRIGGER = 3014;
    static constexpr int EDIT_AREA_TINT_TRIGGER = 3015;

    // ---- Enter Triggers (MOI - hieu ung khi object xuat hien lan dau,
    // KHAC voi Area Trigger, du cung dung chung class EnterEffectObject) ----
    static constexpr int ENTER_MOVE_TRIGGER = 3017;
    static constexpr int ENTER_ROTATE_TRIGGER = 3018;
    static constexpr int ENTER_SCALE_TRIGGER = 3019;
    static constexpr int ENTER_FADE_TRIGGER = 3020;
    static constexpr int ENTER_TINT_TRIGGER = 3021;

    // ---- Art / Change BG-Ground-Midground (DA XAC MINH DUNG qua objects.csv) ----
    static constexpr int CHANGE_BG_TRIGGER = 3029;
    static constexpr int CHANGE_GROUND_TRIGGER = 3030;
    static constexpr int CHANGE_MIDGROUND_TRIGGER = 3031;

    static constexpr int TOUCH_TRIGGER = 1595;

    // ---- Count / Pickup / Item ----
    static constexpr int COUNT_TRIGGER = 1611;
    static constexpr int INSTANT_COUNT_TRIGGER = 1811;
    static constexpr int PICKUP_TRIGGER = 1817;
    static constexpr int RANDOM_TRIGGER = 1912;
    static constexpr int ADVANCED_RANDOM_TRIGGER = 2068;
    static constexpr int SEQUENCE_TRIGGER = 3607;
    static constexpr int SPAWN_PARTICLE_TRIGGER = 3608;
    static constexpr int RESET_TRIGGER = 3618;

    // ---- Camera Triggers ----
    static constexpr int CAMERA_ZOOM_TRIGGER = 1913;
    static constexpr int CAMERA_STATIC_TRIGGER = 1914;
    static constexpr int CAMERA_OFFSET_TRIGGER = 1916;
    static constexpr int CAMERA_GP_OFFSET_TRIGGER = 2901;
    static constexpr int CAMERA_ROTATE_TRIGGER = 2015;
    static constexpr int CAMERA_EDGE_TRIGGER = 2062;
    static constexpr int CAMERA_MODE_TRIGGER = 2925;
    static constexpr int COLLISION_TRIGGER = 1815;
    static constexpr int EDIT_MIDDLEGROUND_TRIGGER = 2999; // doi ten tu MIDGROUND_SETUP_TRIGGER
    static constexpr int BG_SPEED_TRIGGER = 3606;
    static constexpr int MIDGROUND_SPEED_TRIGGER = 3612;
    static constexpr int INSTANT_COLLISION_TRIGGER = 3609;
    static constexpr int ON_DEATH_TRIGGER = 1812;
    static constexpr int PLAYER_CONTROL_TRIGGER = 1932;

    // ---- Color / Gradient / Gravity / Shake / Time Warp ----
    static constexpr int COLOR_TRIGGER = 899;
    static constexpr int GRADIENT_TRIGGER = 2903;
    static constexpr int SHAKE_TRIGGER = 1520;
    static constexpr int GRAVITY_TRIGGER = 2066;
    static constexpr int TIME_WARP_TRIGGER = 1935;
    static constexpr int INVERT_COLOR_TRIGGER = 2921; // MOI

    // ---- Item Edit / Compare ----
    static constexpr int ITEM_EDIT_TRIGGER = 3619;
    static constexpr int ITEM_COMPARE_TRIGGER = 3620;
    static constexpr int REVERSE_TRIGGER = 1917;
    static constexpr int END_TRIGGER = 3600;
    static constexpr int OLD_END_TRIGGER = 1931;         // MOI - End Trigger cu (khac 3600)
    static constexpr int LEVEL_END_TRIGGER_1_0 = 34;     // MOI - trigger legacy tu GD 1.0
    static constexpr int TIME_CONTROL_TRIGGER = 3617;
    static constexpr int TIME_EVENT_TRIGGER = 3615;
    static constexpr int EVENT_TRIGGER = 3604;
    static constexpr int SONG_TRIGGER = 1934;
    static constexpr int EDIT_SONG_TRIGGER = 3605;
    static constexpr int OPTIONS_TRIGGER = 2899;
    static constexpr int PERSISTENT_ITEM_SETUP_TRIGGER = 3641;

    static constexpr int HIDE_PLAYER_TRIGGER = 1612;
    static constexpr int SHOW_PLAYER_TRIGGER = 1613;
    static constexpr int BACKGROUND_EFFECT_ON_TRIGGER = 1818;
    static constexpr int BACKGROUND_EFFECT_OFF_TRIGGER = 1819;
    static constexpr int NO_ENTER_EFFECT_TRIGGER = 1915;
    static constexpr int GAMEPLAY_ROTATION_TRIGGER = 2900;
    static constexpr int TELEPORT_TRIGGER = 3022;
    static constexpr int ENTER_STOP_TRIGGER = 3023;
    static constexpr int AREA_STOP_TRIGGER = 3024;
    static constexpr int OBJECT_CONTROL_TRIGGER = 3655;
    static constexpr int LINK_VISIBLE_TRIGGER = 3662;
    static constexpr int BPM_TRIGGER = 3642;

    // ---- MOI phat hien tu objects.csv (Object-Popup-API) -------------------
    static constexpr int TIME_TRIGGER = 3614;             // Timer Trigger THAT (TimerTriggerGameObject)
    static constexpr int SFX_TRIGGER = 3602;
    static constexpr int EDIT_SFX_TRIGGER = 3603;
    static constexpr int KEYFRAME_ANIMATION_TRIGGER = 3033; // chi 1 loai duy nhat
    static constexpr int UI_TRIGGER = 3613;
    static constexpr int CHECKPOINT = 2063;                // co the khong phai "trigger" dung nghia
    static constexpr int FORCE_BLOCK = 2069;
    static constexpr int FORCE_CIRCLE = 3645;
    static constexpr int SHADER_TRIGGER = 2904;            // shader trigger goc
    static constexpr int SHADER_SHOCK_WAVE = 2905;
    static constexpr int SHADER_SHOCK_LINE = 2907;
    static constexpr int SHADER_GLITCH = 2909;
    static constexpr int SHADER_CHROMATIC_ABERRATION = 2910;
    static constexpr int SHADER_CHROMATIC_GLITCH = 2911;
    static constexpr int SHADER_PIXELATE = 2912;
    static constexpr int KEYFRAME_POINT = 3032; // object dat diem, khac voi KEYFRAME_ANIMATION_TRIGGER (3033)
    static constexpr int SHADER_LENS_CIRCLE = 2913;
    static constexpr int SHADER_RADIAL_BLUR = 2914;
    static constexpr int SHADER_MOTION_BLUR = 2915;
    static constexpr int SHADER_BULGE = 2916;
    static constexpr int SHADER_PINCH = 2917;
    static constexpr int SHADER_GRAYSCALE = 2919;
    static constexpr int SHADER_SEPIA = 2920;
    static constexpr int SHADER_HUE = 2922;
    static constexpr int SHADER_EDIT_COLOR = 2923;
    static constexpr int SHADER_SPLIT_SCREEN = 2924;

    // ---- Helper: nhan dien 1 ID co nam trong dai Shader Trigger khong -------
    // Dai 2904-2924, tru 3 slot bo trong khong ton tai trong game: 2906, 2908, 2918.
    // Dung trong AIGenerate.cpp de gop chung xu ly cho ca 17 loai Shader Trigger
    // (deu dung chung ShaderGameObject) bang 1 dieu kien duy nhat.
    static constexpr bool isShaderTrigger(int id) {
        if (id < SHADER_TRIGGER || id > SHADER_SPLIT_SCREEN) return false; // 2904..2924
        if (id == 2906 || id == 2908 || id == 2918) return false;          // slot bo trong
        return true;
    }

    static constexpr bool isTriggerID(int id) {
        switch (id) {
            case MOVE_TRIGGER: return true;
            case ROTATE_TRIGGER: return true;
            case SCALE_TRIGGER: return true;
            case ALPHA_TRIGGER: return true;
            case TOGGLE_TRIGGER: return true;
            case SPAWN_TRIGGER: return true;
            case ADVANCED_FOLLOW_TRIGGER: return true;
            case PULSE_TRIGGER: return true;
            case STOP_TRIGGER: return true;
            case FOLLOW_TRIGGER: return true;
            case ANIMATE_TRIGGER: return true;
            case FOLLOW_PLAYER_Y_TRIGGER: return true;
            case EDIT_ADVANCED_FOLLOW_TRIGGER: return true;
            case RE_TARGET_ADVANCED_FOLLOW_TRIGGER: return true;
            case AREA_MOVE_TRIGGER: return true;
            case AREA_ROTATE_TRIGGER: return true;
            case AREA_SCALE_TRIGGER: return true;
            case AREA_FADE_TRIGGER: return true;
            case AREA_TINT_TRIGGER: return true;
            case EDIT_AREA_MOVE_TRIGGER: return true;
            case EDIT_AREA_ROTATE_TRIGGER: return true;
            case EDIT_AREA_SCALE_TRIGGER: return true;
            case EDIT_AREA_FADE_TRIGGER: return true;
            case EDIT_AREA_TINT_TRIGGER: return true;
            case ENTER_MOVE_TRIGGER: return true;
            case ENTER_ROTATE_TRIGGER: return true;
            case ENTER_SCALE_TRIGGER: return true;
            case ENTER_FADE_TRIGGER: return true;
            case ENTER_TINT_TRIGGER: return true;
            case CHANGE_BG_TRIGGER: return true;
            case CHANGE_GROUND_TRIGGER: return true;
            case CHANGE_MIDGROUND_TRIGGER: return true;
            case TOUCH_TRIGGER: return true;
            case COUNT_TRIGGER: return true;
            case INSTANT_COUNT_TRIGGER: return true;
            case PICKUP_TRIGGER: return true;
            case RANDOM_TRIGGER: return true;
            case ADVANCED_RANDOM_TRIGGER: return true;
            case SEQUENCE_TRIGGER: return true;
            case SPAWN_PARTICLE_TRIGGER: return true;
            case RESET_TRIGGER: return true;
            case CAMERA_ZOOM_TRIGGER: return true;
            case CAMERA_STATIC_TRIGGER: return true;
            case CAMERA_OFFSET_TRIGGER: return true;
            case CAMERA_GP_OFFSET_TRIGGER: return true;
            case CAMERA_ROTATE_TRIGGER: return true;
            case CAMERA_EDGE_TRIGGER: return true;
            case CAMERA_MODE_TRIGGER: return true;
            case COLLISION_TRIGGER: return true;
            case EDIT_MIDDLEGROUND_TRIGGER: return true;
            case BG_SPEED_TRIGGER: return true;
            case MIDGROUND_SPEED_TRIGGER: return true;
            case INSTANT_COLLISION_TRIGGER: return true;
            case ON_DEATH_TRIGGER: return true;
            case PLAYER_CONTROL_TRIGGER: return true;
            case COLOR_TRIGGER: return true;
            case GRADIENT_TRIGGER: return true;
            case SHAKE_TRIGGER: return true;
            case GRAVITY_TRIGGER: return true;
            case TIME_WARP_TRIGGER: return true;
            case INVERT_COLOR_TRIGGER: return true;
            case ITEM_EDIT_TRIGGER: return true;
            case ITEM_COMPARE_TRIGGER: return true;
            case REVERSE_TRIGGER: return true;
            case END_TRIGGER: return true;
            case OLD_END_TRIGGER: return true;
            case LEVEL_END_TRIGGER_1_0: return true;
            case TIME_CONTROL_TRIGGER: return true;
            case TIME_EVENT_TRIGGER: return true;
            case EVENT_TRIGGER: return true;
            case SONG_TRIGGER: return true;
            case EDIT_SONG_TRIGGER: return true;
            case OPTIONS_TRIGGER: return true;
            case PERSISTENT_ITEM_SETUP_TRIGGER: return true;
            case HIDE_PLAYER_TRIGGER: return true;
            case SHOW_PLAYER_TRIGGER: return true;
            case BACKGROUND_EFFECT_ON_TRIGGER: return true;
            case BACKGROUND_EFFECT_OFF_TRIGGER: return true;
            case NO_ENTER_EFFECT_TRIGGER: return true;
            case GAMEPLAY_ROTATION_TRIGGER: return true;
            case TELEPORT_TRIGGER: return true;
            case ENTER_STOP_TRIGGER: return true;
            case AREA_STOP_TRIGGER: return true;
            case OBJECT_CONTROL_TRIGGER: return true;
            case LINK_VISIBLE_TRIGGER: return true;
            case BPM_TRIGGER: return true;
            case TIME_TRIGGER: return true;
            case SFX_TRIGGER: return true;
            case EDIT_SFX_TRIGGER: return true;
            case KEYFRAME_ANIMATION_TRIGGER: return true;
            case UI_TRIGGER: return true;
            case CHECKPOINT: return true;
            case FORCE_BLOCK: return true;
            case FORCE_CIRCLE: return true;
            case SHADER_TRIGGER: return true;
            case SHADER_SHOCK_WAVE: return true;
            case SHADER_SHOCK_LINE: return true;
            case SHADER_GLITCH: return true;
            case SHADER_CHROMATIC_ABERRATION: return true;
            case SHADER_CHROMATIC_GLITCH: return true;
            case SHADER_PIXELATE: return true;
            case SHADER_LENS_CIRCLE: return true;
            case SHADER_RADIAL_BLUR: return true;
            case SHADER_MOTION_BLUR: return true;
            case SHADER_BULGE: return true;
            case SHADER_PINCH: return true;
            case SHADER_GRAYSCALE: return true;
            case SHADER_SEPIA: return true;
            case SHADER_HUE: return true;
            case SHADER_EDIT_COLOR: return true;
            case SHADER_SPLIT_SCREEN: return true;
            default: return false;
        }
    }
}