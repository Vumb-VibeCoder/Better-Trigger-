#include "GeminiClient.hpp"
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <thread>

using namespace geode::prelude;

namespace ta {

namespace {
    // flash-lite cho nhanh va re, khong can con dao mo trau cho vu nay
    constexpr auto kGeminiModel = "gemini-3.1-flash-lite";
}

void askGemini(std::string const& promptText,
                std::shared_ptr<std::atomic<bool>> alive,
                std::function<void(bool ok, std::string const& textOrError)> onResult) {

    auto apiKey = Mod::get()->getSettingValue<std::string>("gemini-api-key");
    // chi log do dai thoi, log het key ra la ban than luon =))
    geode::log::info("TA: [GeminiClient] apiKey length = {}", apiKey.size());
    if (apiKey.empty()) {
        onResult(false, "Ban quen nhap Gemini API Key trong Settings roi kia, vo dien nhap di.");
        return;
    }

    matjson::Value part;
    part["text"] = promptText;
    matjson::Value content;
    content["parts"] = matjson::Value(std::vector<matjson::Value>{ part });
    matjson::Value body;
    body["contents"] = matjson::Value(std::vector<matjson::Value>{ content });

    std::string url = std::string("https://generativelanguage.googleapis.com/v1beta/models/")
                     + kGeminiModel + ":generateContent";

    std::thread([alive, url, body, apiKey, onResult]() {
        web::WebRequest req;
        req.bodyJSON(body);
        req.header("Content-Type", "application/json");
        req.header("x-goog-api-key", apiKey);

        auto res = req.postSync(url);

        geode::queueInMainThread([alive, res = std::move(res), onResult]() mutable {
            if (!alive->load()) return;

            if (!res.ok()) {
                // server no dang giay hoac key sai, chi cat 200 ky tu dau cho gon,
                // dai qua thi popup no vo tran man hinh xau lam
                std::string detail = "HTTP " + std::to_string(res.code());
                auto b = res.string();
                if (b.isOk() && !b.unwrap().empty()) detail += ": " + b.unwrap().substr(0, 200);
                else if (!res.errorMessage().empty()) detail += " - " + std::string(res.errorMessage());
                onResult(false, detail);
                return;
            }

            auto jsonRes = res.json();
            if (!jsonRes) {
                onResult(false, "Phan hoi tra ve doc khong noi, chac Gemini no dang lag.");
                return;
            }

            auto json = jsonRes.unwrap();
            auto text = json["candidates"][0]["content"]["parts"][0]["text"].asString();
            if (!text.isOk()) {
                // co the bi safety filter chan, hoac Gemini no dinh cong
                onResult(false, "Gemini tra ve rong khong, thu lai xem sao.");
                return;
            }

            onResult(true, text.unwrap());
        });
    }).detach();
}

}