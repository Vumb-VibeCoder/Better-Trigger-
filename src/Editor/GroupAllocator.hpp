#pragma once
// ============================================================================
// Auto ID allocation - tách thành 2 pool riêng biệt:
//   - "Group" pool:  m_groups / m_colorGroups / m_opacityGroups
//     (dùng cho nút Auto Group ID - group riêng của chính block đó)
//   - "Tag" pool:    m_targetGroupID / m_centerGroupID
//     (dùng cho nút Auto Tag ID - group mà trigger nhắm tới)
// Mỗi pool né trùng trong chính pool đó, không quan tâm pool còn lại đang
// dùng số nào.
// ============================================================================

#include <Geode/binding/LevelEditorLayer.hpp>

namespace ta {

// Pool "Group": né group/color-group/opacity-group đang tồn tại.
int findNextFreeGroupId(LevelEditorLayer* editorLayer, int startFrom = 1);
int findFreeGroupId(LevelEditorLayer* editorLayer);

// Pool "Tag": né target-group/center-group đang tồn tại (giá trị
// m_targetGroupID/m_centerGroupID của các trigger hiện có).
int findNextFreeTagId(LevelEditorLayer* editorLayer, int startFrom = 1);
int findFreeTagId(LevelEditorLayer* editorLayer);

}