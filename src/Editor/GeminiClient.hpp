#pragma once
#include <string>
#include <functional>

namespace ta {

// Goi Gemini API voi 1 prompt text, tra ket qua qua callback (chay tren
// main thread). onResult(true, text) neu thanh cong, onResult(false, errorMsg)
// neu that bai. Dung chung cho ca ExplainPopup va FloatingGeminiPanel de
// tranh code trung lap (tung gay bug apiKey doc khac nhau giua 2 noi).
void askGemini(std::string const& promptText,
                std::shared_ptr<std::atomic<bool>> alive,
                std::function<void(bool ok, std::string const& textOrError)> onResult);

}