#pragma once
#include "Functions.h"

namespace logger = SKSE::log;

namespace Functions {
    
    bool preventingHits = false;
    bool onlyPlayerHits = false;
    bool isDelete = false;
    bool hitCheck = false;
    bool explodeCheck = false;
    bool chainExplosions = false;
    bool potionChainExplosions = false;
    bool followersGetAngry = false;

    RE::BGSListForm* explosionFormList;
    RE::BGSListForm* chainableExplosionsFormList;
    RE::TESObjectMISC* marker;
    RE::TESObjectACTI* OilPool;
    RE::TESObjectACTI* CoinExplosionSmall;
    RE::TESObjectACTI* CoinExplosion;
    RE::TESObjectACTI* CoinExplosionLarge;
    RE::SpellItem* BlameSpell;

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

    bool hasDestruction(RE::TESObjectREFRPtr a_target) {
        RE::TESObjectMISC* targetMisc;
        RE::AlchemyItem* targetAlchemy;
        RE::IngredientItem* targetIngredient;

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
            default:
                return false;
                break;
        }
    }
}
