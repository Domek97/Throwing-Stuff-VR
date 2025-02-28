#pragma once

#include <../Source/RE/TESDestructionStageChangedEvent.h>
#include "../Include/BombHandler.h"
#include "../Include/Functions.h"
#include "../Include/Events.h"
#include "../Include/Config.h"


RE::BSEventNotifyControl ObjectEventSink::ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*) {
    // Functions::objectHitId = event->source;
    RE::BGSExplosion* sourceExplosion = RE::TESForm::LookupByID<RE::BGSExplosion>(event->source);
    RE::Actor* causePtr;
    if (event->cause != nullptr) {
        causePtr = RE::TESForm::LookupByID<RE::Actor>(event->cause->formID);
    }
    RE::TESForm* sourceForm = RE::TESForm::LookupByID(event->source);
    // only process onhit and set explodeCheck for items that are destructible
    if (Functions::hasDestruction(event->target) ||
        Functions::HasKeyword(event->target->GetBaseObject(), "VendorItemSoulGem")) {
        #ifdef _DEBUG
        logger::info("onhit {}", event->target->GetFormID());
        #endif
        // if item with gold value is hit and we are preventing hits, set hitcheck
        if (event->source != NULL &&
            (event->target->GetBaseObject()->GetGoldValue() > -1 ||
                Functions::isCoinPurse(event->target->GetBaseObject())) &&
            (Config::preventingHits ||
             (event->cause != nullptr && !event->cause->IsPlayerRef() && Config::onlyPlayerHits)) &&
            (sourceExplosion == nullptr || sourceExplosion->data.damage > 0)) {
            Functions::hitCheck.insert(event->target->formID);
        }
        #ifdef _DEBUG
        else {
            logger::info("hitcheck not set. source {}, target {}, type {}", sourceForm->GetName(),
                            event->target->GetName(), sourceForm->GetFormType());
        }
        #endif
        // if the player hits with a spell, we want the aoe to explode items too. OnDestructionChanged only
        // triggers on items when directly hit by spells
        if (Functions::explosionAllowed(event->source, event->target, event->cause)) {
            if (event->cause == nullptr) {
                BombHandler::Explode(event->target, BombHandler::chainExplosionCause);
            } else {
                if (Config::throwingDispelsInvis) {
                    Functions::DispelInvisibility(event->cause->As<RE::Actor>(), false);
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
            ->CastSpellImmediate(Functions::SilverSpell, false, event->target->As<RE::Actor>(), 1.0f, false, 0.0f,
                                    nullptr);
    } else if (sourceForm != nullptr && Functions::HasKeyword(sourceForm, "isHeartstone")) {
        event->cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
            ->CastSpellImmediate(Functions::HeartSpell, false, event->target->As<RE::Actor>(), 1.0f, false, 0.0f,
                                    nullptr);
        if (event->target->As<RE::Actor>()->GetRace()->HasKeywordString("ActorTypeDwarven")) {
            BombHandler::heartstoneHit = true;
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl ObjectEventSink::ProcessEvent(const RE::TESDestructionStageChangedEvent* event, RE::BSTEventSource<RE::TESDestructionStageChangedEvent>*) {
    // if (!(Functions::onlyExplodeOnHit) || event->target->GetBaseObject()->formID == Functions::objectHitId) {
    #ifdef _DEBUG
    logger::info("destructionstagechanged {}", event->target->GetFormID());
    #endif
    if (!Functions::hitCheck.contains((event->target->formID)) || Functions::explodeCheck) {
        #ifdef _DEBUG
        logger::info("DESTRUCTION!!");
        #endif
        BombHandler::chainExplosionCause = RE::PlayerCharacter::GetSingleton()->As<RE::Actor>();
        if (Config::throwingDispelsInvis &&
            !RE::PlayerCharacter::GetSingleton()->AsMagicTarget()->GetActiveEffectList()->empty()) {
            Functions::DispelInvisibility(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(), false);
        }
        BombHandler::Explode(event->target, RE::PlayerCharacter::GetSingleton()->As<RE::Actor>());
        // Functions::objectHitId = -1;

    }
    #ifdef _DEBUG
    else {
        logger::info("prevented destruction - explodecheck {} hitcheck {}", Functions::explodeCheck,
                        Functions::hitCheck.contains(event->target->formID));
    }
    #endif
    Functions::explodeCheck = false;
    Functions::hitCheck.erase(event->target->formID);

    if (BombHandler::heartstoneHit && Functions::HasKeyword(event->target->As<RE::TESForm>(), "isHeartstone")) {
        BombHandler::DestroyBomb(event->target);
        BombHandler::heartstoneHit = false;
    }
    //}
    return RE::BSEventNotifyControl::kContinue;
}
