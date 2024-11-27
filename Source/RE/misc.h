#pragma once

namespace logger = SKSE::log;

namespace HelperFunctions {
    
    bool preventingHits = false;
    bool onlyPlayerHits = false;
    bool isDelete = false;
    bool hitCheck = false;
    bool explodeCheck = false;
    bool chainExplosions = false;
    bool potionChainExplosions = false;

    static RE::BGSListForm* explosionFormList;
    static RE::BGSListForm* chainableExplosionsFormList;
    static RE::TESObjectMISC* marker;
    static RE::TESObjectACTI* OilPool;
    static RE::TESObjectACTI* CoinExplosionSmall;
    static RE::TESObjectACTI* CoinExplosion;
    static RE::TESObjectACTI* CoinExplosionLarge;

    // a_form: the item whose keywords we're checking
    // a_keyword: the keyword we're comparing against
    bool HasKeyword(RE::TESForm* a_form, std::string a_keyword) {
        const auto keywordForm = a_form->As<RE::BGSKeywordForm>();

        if (keywordForm != NULL) {
            for (RE::BGSKeyword* keyword : keywordForm->GetKeywords()) {
                if (keyword->GetFormEditorID() == a_keyword) {
                    return true;
                }
            }
        }

        return false;
    }

    int CheckObjectKeyword(int formId) {
        if (formId != NULL) {
            RE::TESForm* formToTest = RE::TESForm::LookupByID(formId);

            RE::AlchemyItem* formPotion = RE::TESForm::LookupByID<RE::AlchemyItem>(formId);

            if (HasKeyword(formToTest, "isBlood") && formPotion) return 1;  // blood potion

            if (formPotion) {
                if (formPotion->data.flags.any(RE::AlchemyItem::AlchemyFlag::kPoison) == true) return 11;  // poison

                if (formPotion->data.flags.any(RE::AlchemyItem::AlchemyFlag::kFoodItem) == false) return 12;  // potion
            }

            if (HasKeyword(formToTest, "PurseLarge") || HasKeyword(formToTest, "PurseMedium") ||

                HasKeyword(formToTest, "PurseSmall"))

                return 8;

            if (HasKeyword(formToTest, "Alcohol")) return 0;

            if (HasKeyword(formToTest, "FireBomb")) return 3;

            if (HasKeyword(formToTest, "FrostBomb")) return 5;

            if (HasKeyword(formToTest, "ShockBomb")) return 13;

            if (HasKeyword(formToTest, "PoisonBomb")) return 10;

            if (HasKeyword(formToTest, "OilBomb")) return 9;

            if (HasKeyword(formToTest, "Holybomb")) return 7;

            if (HasKeyword(formToTest, "Materialglass")) return 2;

            if (HasKeyword(formToTest, "nullWhite")) return 15;

            if (HasKeyword(formToTest, "VendorItemSoulGem") && !HasKeyword(formToTest, "ReusableSoulGem")) {
                const auto soulGem = formToTest->As<RE::TESSoulGem>();

                if (soulGem->GetContainedSoul() != RE::SOUL_LEVEL::kNone)

                    return 16;

                else

                    return 17;
            }

            if (HasKeyword(formToTest, "WhiteDustBombBig")) return 18;

            if (HasKeyword(formToTest, "WhiteDustBombSmall")) return 19;

            if (HasKeyword(formToTest, "BlackDustBombBig")) return 20;

            if (HasKeyword(formToTest, "BlackDustBombSmall")) return 21;

            if (HasKeyword(formToTest, "isSoup")) return 22;

            return (-1);
        }

        return (-1);
    }

    // TODO: make real conditions for what determines a potion as detrimental
    bool potionHasNegativeEffects(RE::AlchemyItem* potion) { return true; }

    // potion: the potion whose effects will be applied
    // target: the actor that will drink the potion
    // cause: the actor that caused the potion to explode
    void CastPotion(RE::AlchemyItem* potion, RE::TESObjectREFR& target, RE::TESObjectREFRPtr cause) {
        RE::FormID formID = target.GetFormID();

        RE::Actor* targetActor = RE::TESForm::LookupByID<RE::Actor>(target.formID);
        RE::Actor* causeActor = cause != nullptr ? cause->As<RE::Actor>() : nullptr;
        if (targetActor != NULL) {
            targetActor->DrinkPotion(potion, nullptr);
            if ((cause == nullptr || cause.get()->IsPlayer()) && potionHasNegativeEffects(potion) && !targetActor->IsPlayer()) {
                // TODO: if potion has a negative effect, make target hostile to cause
            }
        }
    }

    // objectRef: the object exploding
    // cause: the actor who caused the object to explode. will be nullptr if called from OnDestructionStageChanged, as only player will be able to do that
    int Explode(RE::TESObjectREFRPtr objectRef, RE::TESObjectREFRPtr cause) {

        RE::TESObjectREFRPtr bomb = objectRef->PlaceObjectAtMe(marker, false);
        bomb.get()->MoveToNode(objectRef.get(), objectRef->Get3D());

        int expType = CheckObjectKeyword(objectRef.get()->GetBaseObject()->GetFormID());

        if (expType != -1) {
            RE::TESBoundObject* explosionRef = explosionFormList->forms[expType]->As<RE::TESBoundObject>();

            if (expType == 11 or expType == 12) {
                RE::AlchemyItem* formPotion = RE::TESForm::LookupByID<RE::AlchemyItem>(objectRef.get()->GetBaseObject()->GetFormID());
                
                RE::TES::GetSingleton()->ForEachReferenceInRange(bomb.get(), 200.0, [&](RE::TESObjectREFR& a_ref) {
                    CastPotion(formPotion, a_ref, cause);
                    return RE::BSContainer::ForEachResult::kContinue;
                });
            }
            bomb->PlaceObjectAtMe(explosionRef, false);
            
            if (expType == 8) {
                if (HasKeyword(objectRef->GetBaseObject(), "PurseLarge")) {
                    objectRef->PlaceObjectAtMe(CoinExplosionLarge, true);
                } else if (HasKeyword(objectRef->GetBaseObject(), "PurseMedium")) {
                    objectRef->PlaceObjectAtMe(CoinExplosion, true);
                } else if (HasKeyword(objectRef->GetBaseObject(), "PurseSmall")) {
                    objectRef->PlaceObjectAtMe(CoinExplosionSmall, true);
                }
            }
            if (expType == 9) {
                bomb->PlaceObjectAtMe(OilPool, true);
                // Papyrus - _SOL_OilPoolScript: MoveToNearestNavMesh()
            }
            if (expType != -1 && expType != 4 && expType != 14) {
                objectRef->Disable();
                objectRef->SetDelete(true);
            }

            // blame player for harming others' belongings
            if (!objectRef->IsAnOwner(RE::PlayerCharacter::GetSingleton(), true, false)) {
                RE::PlayerCharacter::GetSingleton()->StealAlarm(objectRef.get(), objectRef->GetBaseObject(),
                                                                objectRef.get()->extraList.GetCount(), 0,
                                                                objectRef->GetActorOwner(), false);
            }
            return expType;
        }

        return expType;
    }

    // if chaining explosions option is set, return true if we're not chaining potions and formId is not alcohol, blood, potion or poison - or if we are chaining potions
    bool allowChainExplosion(RE::FormID formId) {
        return (chainExplosions && (!potionChainExplosions && formId != chainableExplosionsFormList->forms[0]->GetFormID() &&
                formId != chainableExplosionsFormList->forms[1]->GetFormID() &&
                formId != chainableExplosionsFormList->forms[8]->GetFormID() &&
                formId != chainableExplosionsFormList->forms[9]->GetFormID()) ||
               potionChainExplosions);
    }

    bool isExplosion(RE::FormID formId) { 
        for (RE::TESForm* form : chainableExplosionsFormList->forms) {
            if (form->GetFormID() == formId) {
                return true;
            } 
        }
        return false;
    }

    bool hasDestruction(RE::TESObjectREFRPtr target) {
        RE::TESObjectMISC* targetMisc;
        RE::AlchemyItem* targetAlchemy;
        RE::IngredientItem* targetIngredient;

        switch (target->GetBaseObject()->GetFormType()) {
            case RE::FormType::AlchemyItem:
                targetAlchemy = target->GetBaseObject()->As<RE::AlchemyItem>();
                return targetAlchemy->BGSDestructibleObjectForm::data != nullptr;
                break;
            case RE::FormType::Misc:
                targetMisc = target->GetBaseObject()->As<RE::TESObjectMISC>();
                return targetMisc->BGSDestructibleObjectForm::data != nullptr;
                break;
            case RE::FormType::Ingredient:
                targetIngredient = target->GetBaseObject()->As<RE::IngredientItem>();
                return targetIngredient->BGSDestructibleObjectForm::data != nullptr;
                break;
            default:
                return false;
                break;
        }
    }
}
