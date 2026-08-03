#include "GroupAllocator.hpp"
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/utils/cocos.hpp>
#include <unordered_set>

using namespace geode::prelude;

namespace ta {

int findNextFreeGroupId(LevelEditorLayer* editorLayer, int startFrom) {
    if (!editorLayer) return -1;
    if (startFrom < 1) startFrom = 1;

    std::unordered_set<short> used;

    for (auto obj : CCArrayExt<GameObject*>(editorLayer->m_objects)) {
        if (!obj) continue;

        for (short i = 0; i < obj->m_groupCount; i++) {
            used.insert(obj->m_groups->at(i));
        }
        for (short i = 0; i < obj->m_colorGroupCount; i++) {
            used.insert(obj->m_colorGroups->at(i));
        }
        for (short i = 0; i < obj->m_opacityGroupCount; i++) {
            used.insert(obj->m_opacityGroups->at(i));
        }
    }

    for (int candidate = startFrom; candidate <= 9999; candidate++) {
        if (!used.count((short)candidate)) return candidate;
    }
    return -1; // level dang dung het 1-9999 - cuc ky hiem
}

int findFreeGroupId(LevelEditorLayer* editorLayer) {
    return findNextFreeGroupId(editorLayer, 1);
}

int findNextFreeTagId(LevelEditorLayer* editorLayer, int startFrom) {
    if (!editorLayer) return -1;
    if (startFrom < 1) startFrom = 1;

    std::unordered_set<short> used;

    for (auto obj : CCArrayExt<GameObject*>(editorLayer->m_objects)) {
        if (!obj) continue;

        if (auto eobj = typeinfo_cast<EffectGameObject*>(obj)) {
            if (eobj->m_centerGroupID > 0) used.insert((short)eobj->m_centerGroupID);
            if (eobj->m_targetGroupID > 0) used.insert((short)eobj->m_targetGroupID);
        }
    }

    for (int candidate = startFrom; candidate <= 9999; candidate++) {
        if (!used.count((short)candidate)) return candidate;
    }
    return -1;
}

int findFreeTagId(LevelEditorLayer* editorLayer) {
    return findNextFreeTagId(editorLayer, 1);
}

} // namespace ta