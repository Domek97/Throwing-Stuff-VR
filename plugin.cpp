
#include <../Source/RE/TESDestructionStageChangedEvent.h>
#include "../Include/Functions.h"
#include "../Include/BombHandler.h"

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
        RE::BGSExplosion* sourceExplosion = RE::TESForm::LookupByID<RE::BGSExplosion>(event->source);
        RE::Actor* causePtr;
        if (event->cause != nullptr) {
            causePtr = RE::TESForm::LookupByID<RE::Actor>(event->cause->formID);
        }
        //logger::info("onhit");
        RE::TESForm* sourceForm = RE::TESForm::LookupByID(event->source);
        std::string name = RE::TESForm::LookupByID(event->target->formID)->GetName();
        // only process onhit and set explodeCheck for items that are destructible
        if (Functions::hasDestruction(event->target)) {
            // if item with gold value is hit and we are preventing hits, set hitcheck
            bool test = Functions::isCoinPurse(event->target->GetBaseObject());
            if (event->source != NULL && 
                (event->target->GetBaseObject()->GetGoldValue() > -1 || Functions::isCoinPurse(event->target->GetBaseObject())) && 
                (Functions::preventingHits || (event->cause != nullptr && !event->cause->IsPlayerRef() && Functions::onlyPlayerHits)) && 
                (sourceExplosion == nullptr || sourceExplosion->data.damage > 0)) {
                logger::info("set hitcheck true. source {}, cause , target {}, type {}", sourceForm->GetName(),
                             /*event->cause->GetName(),*/ event->target->GetName(), sourceForm->GetFormType());
                Functions::hitCheck = true;
            } else {
                logger::info("hitcheck not set. source {}, cause , target {}, type {}", sourceForm->GetName(),
                             /*event->cause->GetName(),*/ event->target->GetName(), sourceForm->GetFormType());
            }
            // if the player hits with a spell, we want the aoe to explode items too. OnDestructionChanged only triggers on items when directly hit by spells
            if (!Functions::preventingHits && 
                (!Functions::isExplosion(event->source) || Functions::allowChainExplosion(event->source)) &&
                ((Functions::onlyPlayerHits && event->cause != nullptr && event->cause->IsPlayerRef()) || 
                    !Functions::onlyPlayerHits || Functions::isExplosion(event->source))) {
                    logger::info("explosion");
                    if (event->cause == nullptr) {
                        BombHandler::Explode(event->target, BombHandler::chainExplosionCause);
                    } else {
                        BombHandler::chainExplosionCause = causePtr->As<RE::Actor>();
                        BombHandler::Explode(event->target, causePtr); 
                    }
                }
            // if source explosion doesn't do dmg, onDestructionStageChanged won't be called so don't want to set explodeCheck
                if (!Functions::allowChainExplosion(event->target->formID) && sourceExplosion != nullptr &&
                    Functions::isExplosion(event->source) &&
                sourceExplosion->data.damage > 0) {
                Functions::explodeCheck = true;
            }
            // prevent spell aoes from marking hitcheck
            if (event->cause == nullptr && !Functions::isExplosion(event->source)) {
                Functions::hitCheck = false;
            }
        } else if (Functions::HasKeyword(sourceForm, "MaterialSilver") &&
                   (RE::PlayerCharacter::GetSingleton()->GetEquippedObject(true) == NULL || RE::PlayerCharacter::GetSingleton()->GetEquippedObject(true)->formID != event->source) &&
                   (RE::PlayerCharacter::GetSingleton()->GetEquippedObject(false) == NULL || RE::PlayerCharacter::GetSingleton()->GetEquippedObject(false)->formID != event->source)) {
                event->cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                    ->CastSpellImmediate(Functions::SilverSpell, false, event->target->As<RE::Actor>(), 1.0f, false, 0.0f,
                                         nullptr);
        } else if (Functions::HasKeyword(sourceForm, "isHeartstone")) {
            event->cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                ->CastSpellImmediate(Functions::HeartSpell, false, event->target->As<RE::Actor>(), 1.0f, false, 0.0f, nullptr);
            if (event->target->As<RE::Actor>()->GetRace()->HasKeywordString("ActorTypeDwarven")) {
                BombHandler::heartstoneHit = true;
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
    // End OnHit
    // OnDestructionStageChanged event
    RE::BSEventNotifyControl ProcessEvent(const RE::TESDestructionStageChangedEvent* event,
                                          RE::BSTEventSource<RE::TESDestructionStageChangedEvent>*) {
        logger::info("destructionstagechanged");
        if (Functions::hitCheck || Functions::explodeCheck) {
            logger::info("prevented destruction :( explodecheck {} hitcheck {}", Functions::explodeCheck,
                         Functions::hitCheck);                
        } else {
            logger::info("DESTRUCTION!!");
            BombHandler::chainExplosionCause = RE::PlayerCharacter::GetSingleton()->As<RE::Actor>();
            BombHandler::Explode(event->target, RE::PlayerCharacter::GetSingleton()->As<RE::Actor>());
            if (Functions::throwingDispelsInvis &&
                !RE::PlayerCharacter::GetSingleton()->AsMagicTarget()->GetActiveEffectList()->empty()) {
                //DispelInvisibility(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(), false); 
            }
        }
        Functions::explodeCheck = false;
        Functions::hitCheck = false;

        if (BombHandler::heartstoneHit && Functions::HasKeyword(event->target->As<RE::TESForm>(), "isHeartstone")) {
                BombHandler::DestroyBomb(event->target);
                BombHandler::heartstoneHit = false;
        }
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

    Functions::explosionFormList = RE::TESForm::LookupByEditorID<RE::BGSListForm>("_SOL_ExplosionsFL");
    Functions::chainableExplosionsFormList = RE::TESForm::LookupByEditorID<RE::BGSListForm>("_SOL_ChainableExplosionsFL");
    Functions::marker = RE::TESForm::LookupByEditorID<RE::TESObjectMISC>("_SOL_MarkerBox");
    Functions::OilPool = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_OilPool");
    Functions::CoinExplosion = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_CoinExplosion");
    Functions::CoinExplosionSmall = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_CoinExplosionSmall");
    Functions::CoinExplosionLarge = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>("_SOL_CoinExplosionLarge");
    Functions::BlameSpell = RE::TESForm::LookupByEditorID<RE::SpellItem>("_SOL_BlameSpell");
    Functions::SilverSpell = RE::TESForm::LookupByEditorID<RE::SpellItem>("_SOL_SilverDmgSpell");
    Functions::HeartSpell = RE::TESForm::LookupByEditorID<RE::SpellItem>("_SOL_HeartSpell");
    
    std::string configPath = std::filesystem::current_path().string() + "\\Data\\SKSE\\Plugins\\ThrowingStuffVR.ini";

    CSimpleIniA ini;
    ini.SetUnicode();

    SI_Error rc =
        ini.LoadFile(configPath.c_str());
    if (rc < 0) {
        logger::info("couldn't open INI. config path was: {}", configPath);
    } else {
        logger::info("found INI. Loading values.");
        Functions::preventingHits = ini.GetBoolValue("Settings", "preventHits");
        Functions::onlyPlayerHits = ini.GetBoolValue("Settings", "onlyPlayerHits");
        Functions::onlyPlayerHits = ini.GetBoolValue("Settings", "onlyPlayerHits");
        Functions::chainExplosions = ini.GetBoolValue("Settings", "chainExplosions");
        Functions::potionChainExplosions = ini.GetBoolValue("Settings", "potionChainExplosions");
        Functions::followersGetAngry = ini.GetBoolValue("Settings", "followersGetAngry");
        Functions::breakingIsCrime = ini.GetBoolValue("Settings", "breakingIsCrime");
        Functions::throwingDispelsInvis = ini.GetBoolValue("Settings", "throwingDispelsInvis");
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