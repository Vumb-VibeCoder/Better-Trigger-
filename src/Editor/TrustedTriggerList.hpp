#pragma once
#include <string>

namespace ta {

// Sinh 1 doan text liet ke "ten trigger -> object id" tu chinh du lieu that
// trong TriggerSchema.hpp (khong phai Gemini tu doan). Dung de nhet vao
// system prompt cua "Generate", giup AI chi chon trong tap ID da xac nhan
// dung, thay vi tu nho tu kien thuc train co the sai.
std::string buildTrustedTriggerListForPrompt();

} // namespace ta