#pragma once
#include <string>
#include <vector>

namespace ta {

// Định nghĩa enum Easing thủ công nếu hệ thống Geode của bạn không nhận diện được trực tiếp ở file này
enum class Easing {
    None = 0, EaseInOut, EaseIn, EaseOut, ElasticInOut, ElasticIn, ElasticOut,
    BounceInOut, BounceIn, BounceOut, ExponentialInOut, ExponentialIn, ExponentialOut,
    SineInOut, SineIn, SineOut, BackInOut, BackIn, BackOut
};

enum class ParamType { 
    Float, 
    Int, 
    Bool, 
    Easing, 
    GroupId, 
    ChanceList 
};

struct TriggerParam {
    std::string name;
    ParamType type;
    float minValue;
    float maxValue;
    float defaultValue;
};

struct TriggerSchema {
    int gdObjectId;
    std::string id;
    std::string displayName;
    std::string shortExplanation;
    std::vector<std::string> aliases;
    std::vector<TriggerParam> params;
};

// Khai báo các hàm cốt lõi
std::vector<TriggerSchema>& triggerSchemas();

} // namespace ta