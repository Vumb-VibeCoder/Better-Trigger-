#pragma once

#include <string>
#include <vector>

class EditorUI;
class LevelEditorLayer;
class GameObject;

namespace ta {

// Parse phan JSON Gemini tra ve va spawn cac trigger duoc mo ta vao editor,
// dat gan `selected` (hoac tai 0,0 neu selected la null).
// Tra ve so luong object da tao thanh cong.
// Neu `outCreated` khac null, se duoc dien danh sach cac object vua tao.
int spawnObjectsFromAIJson(EditorUI* ui, LevelEditorLayer* lel,
                            GameObject* selected, std::string const& rawText,
                            std::vector<GameObject*>* outCreated = nullptr);

} // namespace ta