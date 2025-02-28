
#include "../Include/Events.h"
#include "../Include/Config.h"

namespace logger = SKSE::log;

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    logger::info("start init");
    SKSE::Init(skse);
    Config::SetupLog();

    auto* eventSink = ObjectEventSink::GetSingleton();
    auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();

    eventSourceHolder->AddEventSink<RE::TESHitEvent>(eventSink);
    eventSourceHolder->AddEventSink<RE::TESDestructionStageChangedEvent>(eventSink);
    
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            Config::InitValues();
            logger::info("finish init");
        } else if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
            Config::LoadGlobals();
        }
    });
    SKSE::GetPapyrusInterface()->Register(Config::RegisterFuncs);
    return true;
}