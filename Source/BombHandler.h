#pragma once

namespace BombHandler {

    int CheckObjectKeyword(int a_formId);

    // potion: the potion whose effects will be applied
    // target: the actor that will drink the potion
    // cause: the actor that caused the potion to explode
    void CastPotion(RE::AlchemyItem* a_potion, RE::TESObjectREFR& a_target, RE::Actor* a_cause);

    // a_objectRef: the object exploding
    // a_cause: the actor who caused the object to explode
    int Explode(RE::TESObjectREFRPtr a_objectRef, RE::Actor* a_cause);

}