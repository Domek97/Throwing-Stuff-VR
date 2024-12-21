#pragma once
#include "../Include/Functions.h"
#include <unordered_set>

namespace logger = SKSE::log;

namespace Functions {
    
    bool preventingHits = false;
    bool onlyPlayerHits = false;
    bool isDelete = false;
    std::unordered_set<RE::FormID> hitCheck;  // used when preventing hits
    bool explodeCheck = false; // used when preventing chain explosions
    bool chainExplosions = false;
    bool potionChainExplosions = false;
    bool followersGetAngry = false;
    bool breakingIsCrime = false;
    bool throwingDispelsInvis = false;
    //bool onlyExplodeOnHit = false;
    //int objectHitId = -1;

    RE::BGSListForm* explosionFormList;
    RE::BGSListForm* chainableExplosionsFormList;
    RE::TESObjectMISC* marker;
    RE::TESObjectMISC* GoldCoin;
    RE::TESObjectACTI* OilPool;
    RE::SpellItem* BlameSpell;
    RE::SpellItem* SilverSpell;
    RE::SpellItem* HeartSpell;
    RE::BGSExplosion* ImpulseSm;
    RE::BGSExplosion* ImpulseTiny;

    // a_form: the item whose keywords we're checking
    // a_keyword: the keyword we're comparing against
    bool HasKeyword(RE::TESForm* a_form, std::string a_keyword) {
        const auto keywordForm = a_form->As<RE::BGSKeywordForm>();
        std::string name = a_form->GetName();
        if (keywordForm != NULL) {
            std::span<RE::BGSKeyword*> keywords = keywordForm->GetKeywords();
            for (RE::BGSKeyword* keyword : keywords) {
                if (keyword->GetFormEditorID() == a_keyword) {
                    return true;
                }
            }
        }

        return false;
    }

    // if chaining explosions option is set, return true if we're not chaining potions and formId is not alcohol, blood, potion or poison - or if we are chaining potions
    bool allowChainExplosion(RE::FormID a_formId) {
        return (chainExplosions && ((!potionChainExplosions && a_formId != chainableExplosionsFormList->forms[0]->GetFormID() &&
                a_formId != chainableExplosionsFormList->forms[1]->GetFormID() &&
                a_formId != chainableExplosionsFormList->forms[8]->GetFormID() &&
                a_formId != chainableExplosionsFormList->forms[9]->GetFormID()) ||
               potionChainExplosions));
    }

    bool isExplosion(RE::FormID a_formId) { 
        for (RE::TESForm* form : chainableExplosionsFormList->forms) {
            if (form->GetFormID() == a_formId) {
                return true;
            } 
        }
        return false;
    }

    bool isCoinPurse(RE::TESForm* a_form) { 
        return Functions::HasKeyword(a_form, "PurseLarge") || Functions::HasKeyword(a_form, "PurseMedium") ||
                Functions::HasKeyword(a_form, "PurseSmall") || Functions::HasKeyword(a_form, "onmoPurse");
    }
    
    bool hasDestruction(RE::TESObjectREFRPtr a_target) {
        RE::TESObjectMISC* targetMisc;
        RE::AlchemyItem* targetAlchemy;
        RE::IngredientItem* targetIngredient;
        RE::TESFlora* targetFlora;
        RE::TESSoulGem* targetSG;

        switch (a_target->GetBaseObject()->GetFormType()) {
            case RE::FormType::AlchemyItem:
                targetAlchemy = a_target->GetBaseObject()->As<RE::AlchemyItem>();
                return targetAlchemy->BGSDestructibleObjectForm::data != nullptr;
                break;
            case RE::FormType::Misc:
                targetMisc = a_target->GetBaseObject()->As<RE::TESObjectMISC>();
                return targetMisc->BGSDestructibleObjectForm::data != nullptr;
                break;
            case RE::FormType::Ingredient:
                targetIngredient = a_target->GetBaseObject()->As<RE::IngredientItem>();
                return targetIngredient->BGSDestructibleObjectForm::data != nullptr;
                break;
            case RE::FormType::Flora:
                targetFlora = a_target->GetBaseObject()->As<RE::TESFlora>();
                return targetFlora->BGSDestructibleObjectForm::data != nullptr;
            case RE::FormType::SoulGem:
                targetSG = a_target->GetBaseObject()->As<RE::TESSoulGem>();
                return targetSG->BGSDestructibleObjectForm::data != nullptr;
            default:
                return false;
                break;
        }
    }
}
