#pragma once

#include "../Include/BombHandler.h"
#include "../Include/Functions.h"

namespace BombHandler {
    bool heartstoneHit = false;
    RE::Actor* chainExplosionCause;

    int CheckObjectKeyword(int a_formId) {
        if (a_formId != NULL) {
            RE::TESForm* formToTest = RE::TESForm::LookupByID(a_formId);

            RE::AlchemyItem* formPotion = RE::TESForm::LookupByID<RE::AlchemyItem>(a_formId);

            if (Functions::HasKeyword(formToTest, "isBlood") && formPotion) return 1;  // blood potion

            if (formPotion) {
                if (formPotion->data.flags.any(RE::AlchemyItem::AlchemyFlag::kPoison) == true ||
                    Functions::HasKeyword(formToTest, "Poison"))
                    return 11;  // poison

                if (formPotion->data.flags.any(RE::AlchemyItem::AlchemyFlag::kFoodItem) == false ||
                    Functions::HasKeyword(formToTest, "Potion"))
                    return 12;  // potion
            }

            if (Functions::HasKeyword(formToTest, "PurseLarge") || Functions::HasKeyword(formToTest, "PurseMedium") ||
                Functions::HasKeyword(formToTest, "PurseSmall") || Functions::HasKeyword(formToTest, "onmoPurse"))

                return 8;

            if (Functions::HasKeyword(formToTest, "Alcohol")) return 0;

            if (Functions::HasKeyword(formToTest, "FireBomb")) return 3;

            if (Functions::HasKeyword(formToTest, "FrostBomb")) return 5;

            if (Functions::HasKeyword(formToTest, "ShockBomb")) return 13;

            if (Functions::HasKeyword(formToTest, "PoisonBomb")) return 10;

            if (Functions::HasKeyword(formToTest, "OilBomb")) return 9;

            if (Functions::HasKeyword(formToTest, "Holybomb")) return 7;

            if (Functions::HasKeyword(formToTest, "MaterialGlass")) return 2;

            if (Functions::HasKeyword(formToTest, "nullWhite")) return 15;

            if (Functions::HasKeyword(formToTest, "VendorItemSoulGem") &&
                !Functions::HasKeyword(formToTest, "ReusableSoulGem")) {
                const auto soulGem = formToTest->As<RE::TESSoulGem>();
                if (soulGem->GetContainedSoul() != RE::SOUL_LEVEL::kNone)
                    return 16;
                else
                    return 17;
            }

            if (Functions::HasKeyword(formToTest, "WhiteDustBombBig")) return 18;

            if (Functions::HasKeyword(formToTest, "WhiteDustBombSmall")) return 19;

            if (Functions::HasKeyword(formToTest, "BlackDustBombBig")) return 20;

            if (Functions::HasKeyword(formToTest, "BlackDustBombSmall")) return 21;

            if (Functions::HasKeyword(formToTest, "isSoup")) return 22;

            return (-1);
        }

        return (-1);
    }

    // potion: the potion whose effects will be applied
    // target: the actor that will drink the potion
    // cause: the actor that caused the potion to explode
    void CastPotion(RE::AlchemyItem* a_potion, RE::TESObjectREFR& a_target, RE::Actor* a_cause) {
        RE::FormID formID = a_target.GetFormID();

        RE::Actor* targetActor = a_target.As<RE::Actor>();
        if (targetActor != NULL && a_cause != NULL) {
            a_cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                ->CastSpellImmediate(a_potion, false, targetActor, 1.0f, false, 0.0f, nullptr);
            if (a_cause->IsPlayerRef() && !targetActor->IsPlayerRef() &&
                (!targetActor->IsPlayerTeammate() || Functions::followersGetAngry) && a_potion->hostileCount > 0) {
                a_cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                    ->CastSpellImmediate(Functions::BlameSpell, false, targetActor, 1.0f, false, 0.0f, nullptr);
            }
        }
    }

    void DestroyBomb(RE::TESObjectREFRPtr a_objectRef) {
        a_objectRef->Disable();
        a_objectRef->SetDelete(true);
        // blame player for harming others' belongings
        if (!a_objectRef->IsAnOwner(RE::PlayerCharacter::GetSingleton(), true, false) && Functions::breakingIsCrime) {
            RE::PlayerCharacter::GetSingleton()->StealAlarm(a_objectRef.get(), a_objectRef->GetBaseObject(),
                                                            a_objectRef.get()->extraList.GetCount(), 0,
                                                            a_objectRef->GetActorOwner(), false);
        }
    }

    // a_objectRef: the object exploding
    // a_cause: the actor who caused the object to explode
    int Explode(RE::TESObjectREFRPtr a_objectRef, RE::Actor* a_cause) {
            RE::TESObjectREFRPtr bomb = a_objectRef->PlaceObjectAtMe(Functions::marker, false);
            bomb.get()->MoveToNode(a_objectRef.get(), a_objectRef->Get3D());

            int expType = CheckObjectKeyword(a_objectRef.get()->GetBaseObject()->GetFormID());

            if (expType != -1) {
                RE::TESBoundObject* explosionRef =
                Functions::explosionFormList->forms[expType]->As<RE::TESBoundObject>();

                if (expType == 11 or expType == 12) {
                    RE::AlchemyItem* formPotion =
                        RE::TESForm::LookupByID<RE::AlchemyItem>(a_objectRef.get()->GetBaseObject()->GetFormID());

                    RE::TES::GetSingleton()->ForEachReferenceInRange(bomb.get(), 200.0, [&](RE::TESObjectREFR& a_ref)
                    {
                        CastPotion(formPotion, a_ref, a_cause);
                        return RE::BSContainer::ForEachResult::kContinue;
                    });
                }
                bomb->PlaceObjectAtMe(explosionRef, false);

                // process extra functions for particular explosions

                std::random_device rd;
                std::mt19937 gen(rd());
                int i = 0;
                int amount;
                switch (expType) {
                    case 0:  // alcohol
                        RE::TES::GetSingleton()->ForEachReferenceInRange(bomb.get(), 200.0, [&](RE::TESObjectREFR& a_ref) {
                                RE::Actor* targetActor = a_ref.As<RE::Actor>();
                                if (targetActor != nullptr && a_cause->IsPlayerRef() && !targetActor->IsPlayerRef() &&
                                    (!targetActor->IsPlayerTeammate() || Functions::followersGetAngry)) {
                                    a_cause->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                                        ->CastSpellImmediate(Functions::BlameSpell, false, targetActor, 1.0f, false,0.0f, nullptr);
                                }
                                return RE::BSContainer::ForEachResult::kContinue;
                            });
                        break;
                    case 8:  // coin
                        if (Functions::HasKeyword(a_objectRef->GetBaseObject(), "onmoPurse")) {
                            std::string name = a_objectRef->GetDisplayFullName();
                            amount = atoi(name.substr(name.find_first_of("(") + 1, (name.find_first_of(")") - name.find_first_of("("))).c_str());
                        } else if (Functions::HasKeyword(a_objectRef->GetBaseObject(), "PurseLarge")) {
                            std::uniform_int_distribution<> distrib(20, 60);
                            amount = distrib(gen);
                        } else if (Functions::HasKeyword(a_objectRef->GetBaseObject(), "PurseMedium")) {
                            std::uniform_int_distribution<> distrib(10, 40);
                            amount = distrib(gen);
                        } else if (Functions::HasKeyword(a_objectRef->GetBaseObject(), "PurseSmall")) {
                            std::uniform_int_distribution<> distrib(5, 20);
                            amount = distrib(gen);
                        }
                        while (i < amount) {
                            a_objectRef->PlaceObjectAtMe(Functions::GoldCoin, true);
                            i++;
                        }
                        a_objectRef->PlaceObjectAtMe(Functions::ImpulseSm, false);
                        break;
                    case 9:  // dwarven oil
                        bomb->PlaceObjectAtMe(Functions::OilPool, true);
                        // Papyrus - _SOL_OilPoolScript: MoveToNearestNavMesh()
                        break;
                }
                if (expType != 4 && expType != 14) {
                    DestroyBomb(a_objectRef);
                    if (expType != 8) { // coin already has impulse, doesn't need this one
                        bomb->PlaceObjectAtMe(Functions::ImpulseTiny, false);
                    }
                }
            }
            return expType;
        }
}