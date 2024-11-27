#pragma once

namespace logger = SKSE::log;

namespace Functions {
    
    extern bool preventingHits;
    extern bool onlyPlayerHits;
    extern bool isDelete;
    extern bool hitCheck;
    extern bool explodeCheck;
    extern bool chainExplosions;
    extern bool potionChainExplosions;
    extern bool followersGetAngry;

    extern RE::BGSListForm* explosionFormList;
    extern RE::BGSListForm* chainableExplosionsFormList;
    extern RE::TESObjectMISC* marker;
    extern RE::TESObjectACTI* OilPool;
    extern RE::TESObjectACTI* CoinExplosionSmall;
    extern RE::TESObjectACTI* CoinExplosion;
    extern RE::TESObjectACTI* CoinExplosionLarge;
    extern RE::SpellItem* BlameSpell;

    // a_form: the item whose keywords we're checking
    // a_keyword: the keyword we're comparing against
    bool HasKeyword(RE::TESForm* a_form, std::string a_keyword);

    // if chaining explosions option is set, return true if we're not chaining potions and formId is not alcohol, blood, potion or poison - or if we are chaining potions
    bool allowChainExplosion(RE::FormID a_formId);

    bool isExplosion(RE::FormID a_formId);

    bool hasDestruction(RE::TESObjectREFRPtr a_target);
}
