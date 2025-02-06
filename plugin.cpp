
#include <../Source/RE/TESDestructionStageChangedEvent.h>
#include "../Include/Functions.h"
#include "../Include/BombHandler.h"

#include <spdlog/sinks/basic_file_sink.h>
#include<SimpleIni.h>

namespace logger = SKSE::log;

void DispelInvisibility(RE::Actor* a_actor, bool a_force) {
    std::vector<RE::ActiveEffect*> queued;
    a_actor->AsMagicTarget()->VisitActiveEffects([&](RE::ActiveEffect* effect) {
        const auto setting = effect ? effect->GetBaseObject() : nullptr;
        if (setting && setting->HasArchetype(RE::EffectArchetypes::ArchetypeID::kInvisibility)) {
            queued.push_back(effect);
        }
        return RE::BSContainer::ForEachResult::kContinue;
    });
    for (const auto& effect : queued) {
        effect->Dispel(a_force);
    }
    
}

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
    
    RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*) {
            //Functions::objectHitId = event->source; 
            RE::BGSExplosion* sourceExplosion = RE::TESForm::LookupByID<RE::BGSExplosion>(event->source);
            std::string test = sourceExplosion != nullptr ? sourceExplosion->GetFullName() : "null";
            std::string target = event->target->GetDisplayFullName();
            RE::Actor* causePtr;
            if (event->cause != nullptr) {
                causePtr = RE::TESForm::LookupByID<RE::Actor>(event->cause->formID);
            }
            RE::TESForm* sourceForm = RE::TESForm::LookupByID(event->source);
            // only process onhit and set explodeCheck for items that are destructible
            if (Functions::hasDestruction(event->target) || Functions::HasKeyword(event->target->GetBaseObject(), "VendorItemSoulGem")) {
                logger::info("onhit {}", event->target->GetFormID());
                // if item with gold value is hit and we are preventing hits, set hitcheck
                bool test = Functions::isCoinPurse(event->target->GetBaseObject());
                if (event->source != NULL &&
                    (event->target->GetBaseObject()->GetGoldValue() > -1 ||
                     Functions::isCoinPurse(event->target->GetBaseObject())) &&
                    (Functions::preventingHits ||
                     (event->cause != nullptr && !event->cause->IsPlayerRef() && Functions::onlyPlayerHits)) &&
                    (sourceExplosion == nullptr || sourceExplosion->data.damage > 0)) {
                    //logger::info("set hitcheck true. source {}, target {}, type {}", sourceForm->GetName(), event->target->GetName(), sourceForm->GetFormType());
                    Functions::hitCheck.insert(event->target->formID);
                } else {
                    logger::info("hitcheck not set. source {}, target {}, type {}", sourceForm->GetName(), event->target->GetName(), sourceForm->GetFormType());
                }
                // if the player hits with a spell, we want the aoe to explode items too. OnDestructionChanged only
                // triggers on items when directly hit by spells
                if (!(Functions::preventingHits && !Functions::isExplosion(event->source)) &&
                    ((Functions::isExplosion(event->source) && Functions::allowChainExplosion(event->source)) || Functions::HasKeyword(event->target->GetBaseObject(), "VendorItemSoulGem")) &&
                    ((Functions::onlyPlayerHits && event->cause == nullptr || event->cause->IsPlayerRef()) ||
                     !Functions::onlyPlayerHits)) {
                    //logger::info("explosion");
                    if (event->cause == nullptr) {
                        BombHandler::Explode(event->target, BombHandler::chainExplosionCause);
                    } else {
                        if (Functions::throwingDispelsInvis) {
                            DispelInvisibility(event->cause->As<RE::Actor>(), false);
                        }
                        BombHandler::chainExplosionCause = causePtr->As<RE::Actor>();
                        BombHandler::Explode(event->target, causePtr);
                    }
                }
                // if source explosion doesn't do dmg, onDestructionStageChanged won't be called so don't want to set
                // explodeCheck
                if (!Functions::allowChainExplosion(event->target->formID) && sourceExplosion != nullptr &&
                    Functions::isExplosion(event->source) && sourceExplosion->data.damage > 0) {
                    Functions::explodeCheck = true;
                }
                // prevent spell aoes from marking hitcheck
                if (event->cause == nullptr && !Functions::isExplosion(event->source)) {
                    Functions::hitCheck.erase(event->target->formID);
                }
            } else if (sourceForm != nullptr &&
                       (Functions::HasKeyword(sourceForm, "WeaponMaterialSilver") ||
                        Functions::HasKeyword(sourceForm, "MaterialSilver") ||
                        Functions::HasKeyword(sourceForm, "MaterialHoly")) &&
                       (RE::PlayerCharacter::GetSingleton()->GetEquippedObject(true) == NULL ||
                        RE::PlayerCharacter::GetSingleton()->GetEquippedObject(true)->formID != event->source) &&
                       (RE::PlayerCharacter::GetSingleton()->GetEquippedObject(false) == NULL ||
                        RE::PlayerCharacter::GetSingleton()->GetEquippedObject(false)->formID != event->source)) {
                event->cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                    ->CastSpellImmediate(Functions::SilverSpell, false, event->target->As<RE::Actor>(), 1.0f, false,
                                         0.0f, nullptr);
            } else if (sourceForm != nullptr && Functions::HasKeyword(sourceForm, "isHeartstone")) {
                event->cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                    ->CastSpellImmediate(Functions::HeartSpell, false, event->target->As<RE::Actor>(), 1.0f, false,
                                         0.0f, nullptr);
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
        //if (!(Functions::onlyExplodeOnHit) || event->target->GetBaseObject()->formID == Functions::objectHitId) {
            logger::info("destructionstagechanged {}", event->target->GetFormID());
            if (Functions::hitCheck.contains((event->target->formID)) || Functions::explodeCheck) {
                logger::info("prevented destruction - explodecheck {} hitcheck {}", Functions::explodeCheck,
                             Functions::hitCheck.contains(event->target->formID));
            } else {
                logger::info("DESTRUCTION!!");
                BombHandler::chainExplosionCause = RE::PlayerCharacter::GetSingleton()->As<RE::Actor>();
                if (Functions::throwingDispelsInvis &&
                    !RE::PlayerCharacter::GetSingleton()->AsMagicTarget()->GetActiveEffectList()->empty()) {
                    DispelInvisibility(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(), false);
                }
                BombHandler::Explode(event->target, RE::PlayerCharacter::GetSingleton()->As<RE::Actor>());
                //Functions::objectHitId = -1;
            }
            Functions::explodeCheck = false;
            Functions::hitCheck.erase(event->target->formID);

            if (BombHandler::heartstoneHit &&
                Functions::HasKeyword(event->target->As<RE::TESForm>(), "isHeartstone")) {
                BombHandler::DestroyBomb(event->target);
                BombHandler::heartstoneHit = false;
            }
        //}
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
    Functions::BlameSpell = RE::TESForm::LookupByEditorID<RE::SpellItem>("_SOL_BlameSpell");
    Functions::SilverSpell = RE::TESForm::LookupByEditorID<RE::SpellItem>("_SOL_SilverDmgSpell");
    Functions::HeartSpell = RE::TESForm::LookupByEditorID<RE::SpellItem>("_SOL_HeartSpell");
    Functions::GoldCoin = RE::TESForm::LookupByEditorID<RE::TESObjectMISC>("Gold001");
    Functions::ImpulseSm = RE::TESForm::LookupByEditorID<RE::BGSExplosion>("_SOL_ImpulseSm");
    Functions::ImpulseTiny = RE::TESForm::LookupByEditorID<RE::BGSExplosion>("_SOL_ImpulseTiny");

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
        //Functions::onlyExplodeOnHit = ini.GetBoolValue("Settings", "onlyExplodeOnHit");
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