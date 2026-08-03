#include <Geode/loader/Mod.hpp>
#include <Geode/loader/Log.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("[TriggerAssistant] Loaded - Phase A (rule-based, offline) knowledge base ready.");
}
