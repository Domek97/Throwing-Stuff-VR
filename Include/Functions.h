#pragma once
#include <unordered_set>

namespace logger = SKSE::log;

namespace Functions {
    
    extern bool preventingHits;
    extern bool onlyPlayerHits;
    extern bool isDelete;
    extern std::unordered_set<RE::FormID> hitCheck;  // used when preventing hits
    extern bool explodeCheck;
    extern bool chainExplosions;
    extern bool potionChainExplosions;
    extern bool followersGetAngry;
    extern bool breakingIsCrime;
    extern bool throwingDispelsInvis;
    //extern bool onlyExplodeOnHit;
    //extern int objectHitId;

    extern RE::BGSListForm* explosionFormList;
    extern RE::BGSListForm* chainableExplosionsFormList;
    extern RE::TESObjectMISC* marker;
    extern RE::TESObjectMISC* GoldCoin;
    extern RE::TESObjectACTI* OilPool;
    extern RE::BGSExplosion* ImpulseSm;
    extern RE::BGSExplosion* ImpulseTiny;

    extern RE::SpellItem* BlameSpell;
    extern RE::SpellItem* SilverSpell;
    extern RE::SpellItem* HeartSpell;

    // a_form: the item whose keywords we're checking
    // a_keyword: the keyword we're comparing against
    bool HasKeyword(RE::TESForm* a_form, std::string a_keyword);
    // if chaining explosions option is set, return true if we're not chaining potions and formId is not alcohol, blood, potion or poison - or if we are chaining potions
    bool allowChainExplosion(RE::FormID a_formId);

    bool isExplosion(RE::FormID a_formId);
    bool isCoinPurse(RE::TESForm* a_form); 

    bool hasDestruction(RE::TESObjectREFRPtr a_target);
}
