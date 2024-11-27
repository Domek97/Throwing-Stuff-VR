
#include <../Source/RE/TESDestructionStageChangedEvent.h>
#include <../Source/RE/misc.h>
#include <spdlog/sinks/basic_file_sink.h>
#include<SimpleIni.h>

namespace logger = SKSE::log;

// Event handler class
class ObjectEventSink : public RE::BSTEventSink<RE::TESHitEvent>,
                        public RE::BSTEventSink<RE::TESDestructionStageChangedEvent> {
    ObjectEventSink() = default;

    ObjectEventSink(const ObjectEventSink&) = delete;

    ObjectEventSink(ObjectEventSink&&) = delete;

    ObjectEventSink& operator=(const ObjectEventSink&) = delete;

    ObjectEventSink& operator=(ObjectEventSink&&) = delete;

public:
    static ObjectEventSink* GetSingleton() {
        static ObjectEventSink singleton;

        return &singleton;
    }


    // OnHit event
    // TODO: get the game to recognize onhits for potions and melee attacks

    RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*) {

        // only process onhit and set explodeCheck for items that are destructible
        if (HelperFunctions::hasDestruction(event->target)) {
            RE::TESForm* sourceForm = RE::TESForm::LookupByID(event->source);
            // if item with gold value is hit by player and we are preventing hits, set hitcheck
            if (HelperFunctions::preventingHits &&
                (event->target->GetBaseObject()->GetGoldValue() > -1 && event->cause != nullptr &&
                 event->cause->IsPlayerRef())/* &&
                sourceForm->GetName() == NULL*/) {
                logger::info("set hitcheck true. source {}, cause {}, target {}, type {}", sourceForm->GetName(),
                             event->cause->GetName(), event->target->GetName(), sourceForm->GetFormType());
                HelperFunctions::hitCheck = true;
                // TODO: clear object's destruction
                event->target->DamageObject(-2.0, true); // THIS DOESN'T WORK - DON'T CURRENTLY KNOW A WAY TO PREVENT DESTRUCTION
            }
            // if the player hits with a spell, we want the aoe to explode items too. OnDestructionChanged only triggers on items when directly hit by spells
            if (HelperFunctions::allowChainExplosion(event->source)) {
                if (!HelperFunctions::preventingHits &&
                    !(HelperFunctions::onlyPlayerHits && event->cause != nullptr &&
                      !event->cause->IsPlayer())) {
                    HelperFunctions::Explode(event->target, event->cause); 
                }
            }

            RE::BGSExplosion* sourceExplosion = RE::TESForm::LookupByID<RE::BGSExplosion>(event->source);
            // if source explosion doesn't do dmg, onDestructionStageChanged won't be called so don't want to set explodeCheck
            if (!HelperFunctions::allowChainExplosion(event->target->formID) && sourceExplosion != nullptr && sourceExplosion->data.damage > 0) {
                HelperFunctions::explodeCheck = true;
            }
            // prevent spell aoes from marking hitcheck
            if (event->cause == nullptr && !HelperFunctions::isExplosion(event->source)) {
                HelperFunctions::hitCheck = false;
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
    // End OnHit

    // OnDestructionStageChanged event
    RE::BSEventNotifyControl ProcessEvent(const RE::TESDestructionStageChangedEvent* event,
                                          RE::BSTEventSource<RE::TESDestructionStageChangedEvent>*) {
        if (event->newStage == 1) {
            if ((HelperFunctions::preventingHits && HelperFunctions::hitCheck) || (!HelperFunctions::chainExplosions && HelperFunctions::explodeCheck)) {
                logger::info("prevented destruction :( explodecheck {} hitcheck {}", HelperFunctions::explodeCheck, HelperFunctions::hitCheck);                
            } else {
                logger::info("DESTRUCTION!!");
                HelperFunctions::Explode(event->target, nullptr);
            }
        }
        HelperFunctions::explodeCheck = false;
        HelperFunctions::hitCheck = false;
        return RE::BSEventNotifyControl::kContinue;
    }
    // End OnDestructionStageChanged
};

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();

    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");

    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();

    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);

    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);

    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));

    spdlog::set_default_logger(std::move(loggerPtr));

    spdlog::set_level(spdlog::level::trace);

    spdlog::flush_on(spdlog::level::trace);
}

// Initialize forms we need for the entire session
void InitValues() {
    logger::info("Initializing values.");

    HelperFunctions::explosionFormList = RE::TESForm::LookupByEditorID<RE::BGSListForm>("_SOL_ExplosionsFL");
    HelperFunctions::chainableExplosionsFormList = RE::TESForm::LookupByEditorID<RE::BGSListForm>("_SOL_ChainableExplosionsFL");
    HelperFunctions::marker = RE::TESForm::LookupByEditorID<RE::TESObjectMISC>("_SOL_MarkerBox");
    HelperFunctions::OilPool = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_OilPool");
    HelperFunctions::CoinExplosion = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_CoinExplosion");
    HelperFunctions::CoinExplosionSmall = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_CoinExplosionSmall");
    HelperFunctions::CoinExplosionLarge = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_CoinExplosionLarge");
    std::string configPath = std::filesystem::current_path().string() + "\\Data\\SKSE\\Plugins\\ThrowingStuffVR.ini";

    CSimpleIniA ini;
    ini.SetUnicode();

    SI_Error rc =
        ini.LoadFile(configPath.c_str());
    if (rc < 0) {
        logger::info("couldn't open INI. config path was: {}", configPath);
    } else {
        logger::info("found INI. Loading values.");
        HelperFunctions::preventingHits = ini.GetBoolValue("Settings", "preventHits");
        HelperFunctions::onlyPlayerHits = ini.GetBoolValue("Settings", "onlyPlayerHits");
        HelperFunctions::onlyPlayerHits = ini.GetBoolValue("Settings", "onlyPlayerHits");
        HelperFunctions::chainExplosions = ini.GetBoolValue("Settings", "chainExplosions");
        HelperFunctions::potionChainExplosions = ini.GetBoolValue("Settings", "potionChainExplosions");

    }
}

// TODO: use MCM helper to change ini file and variables when changing mcm options
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    logger::info("start init");
    SKSE::Init(skse);
    SetupLog();

    auto* eventSink = ObjectEventSink::GetSingleton();
    auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();

    eventSourceHolder->AddEventSink<RE::TESHitEvent>(eventSink);
    eventSourceHolder->AddEventSink<RE::TESDestructionStageChangedEvent>(eventSink);

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            InitValues();
            logger::info("finish init");
        }
    });

    return true;
}