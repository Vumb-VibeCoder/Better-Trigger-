#include "PlacementAssist.hpp"
#include "GroupAllocator.hpp"
#include "../Knowledge/TriggerSchema.hpp"
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Loader.hpp>
#include <unordered_map>
#include <string>

using namespace geode::prelude;

namespace ta {

bool g_autoGroupEnabled = false;
bool g_autoTagEnabled = false;
bool g_copyFirstEnabled = false;
bool g_suppressPlacementAssist = false;

namespace {
    std::unordered_map<std::string, Ref<GameObject>> g_firstOfType;

    void copyLiveParams(EffectGameObject* from, GameObject* fromObj,
                         EffectGameObject* to, GameObject* toObj) {
        to->m_duration = from->m_duration;
        to->m_moveOffset = ccp(from->m_moveOffset.x, from->m_moveOffset.y);
        to->m_easingType = from->m_easingType;
        to->m_rotationDegrees = from->m_rotationDegrees;
        to->m_opacity = from->m_opacity;
        to->m_fadeInDuration = from->m_fadeInDuration;
        to->m_holdDuration = from->m_holdDuration;
        to->m_fadeOutDuration = from->m_fadeOutDuration;
        to->m_followXMod = from->m_followXMod;
        to->m_followYMod = from->m_followYMod;
        to->m_spawnTriggerDelay = from->m_spawnTriggerDelay;
        to->m_activateGroup = from->m_activateGroup;
        toObj->m_scaleX = fromObj->m_scaleX;
        toObj->m_scaleY = fromObj->m_scaleY;
    }
}

void resetTemplates() {
    g_firstOfType.clear();
}

bool onObjectPlaced(GameObject* obj, LevelEditorLayer* lel) {
    if (!obj || !lel) return false;
    if (g_suppressPlacementAssist) return false; // AI (hoặc nguồn khác) đang tự lo, không đụng vào
    if (!g_autoGroupEnabled && !g_autoTagEnabled && !g_copyFirstEnabled) return false;

    bool changed = false;

    // ------------------------------------------------------------------
    // 1) Auto Group ID: gán GROUP ID (obj->addToGroup) - pool RIÊNG, chỉ né
    // các group/color-group/opacity-group đang tồn tại. KHÔNG né target
    // group/center group của trigger - xem GroupAllocator để biết vì sao.
    // ------------------------------------------------------------------
    if (g_autoGroupEnabled) {
        int freeGroupId = findFreeGroupId(lel);
        geode::log::info("TA: [AutoGroup] objectID={} -> group={}", obj->m_objectID, freeGroupId);
        if (freeGroupId > 0) {
            obj->addToGroup(freeGroupId);
            changed = true;
        }
    }

    const TriggerSchema* schema = nullptr;
    for (auto& s : triggerSchemas()) {
        if (s.gdObjectId == obj->m_objectID) { schema = &s; break; }
    }
    if (!schema) return changed;

    auto eff = typeinfo_cast<EffectGameObject*>(obj);
    if (!eff) return changed;

    // ------------------------------------------------------------------
    // 2) Auto Tag ID: gán TARGET GROUP ID (m_targetGroupID) - pool RIÊNG,
    // chỉ né các target-group/center-group đang tồn tại, không quan tâm
    // group thường.
    // ------------------------------------------------------------------
    if (g_autoTagEnabled) {
        int freeTagId = findFreeTagId(lel);
        geode::log::info("TA: [AutoTag] schema={} -> targetGroupID={}", schema->id, freeTagId);
        if (freeTagId > 0) {
            eff->m_targetGroupID = freeTagId;
            if (eff->m_objectLabel) {
                eff->m_objectLabel->setString(std::to_string(freeTagId).c_str());
            }
            changed = true;
        }
    }

    // ------------------------------------------------------------------
    // 3) Copy First
    // ------------------------------------------------------------------
    if (g_copyFirstEnabled) {
        auto firstIt = g_firstOfType.find(schema->id);
        if (firstIt == g_firstOfType.end() || !firstIt->second) {
            g_firstOfType[schema->id] = Ref<GameObject>(obj);
            geode::log::info("TA: [CopyFirst] stored as FIRST of type {} (ptr={})", schema->id, (void*)obj);
        } else {
            auto firstObj = firstIt->second.data();
            if (auto firstEff = typeinfo_cast<EffectGameObject*>(firstObj)) {
                copyLiveParams(firstEff, firstObj, eff, obj);
                changed = true;
            }
        }
    }

    return changed;
}

} // namespace ta