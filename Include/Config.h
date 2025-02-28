#pragma once
#include <SimpleIni.h>

namespace Config {
    extern CSimpleIniA ini;

    extern bool preventingHits;
    extern bool onlyPlayerHits;
    extern bool chainExplosions;
    extern bool potionChainExplosions;
    extern bool followersGetAngry;
    extern bool breakingIsCrime;
    extern bool throwingDispelsInvis;
    // extern bool onlyExplodeOnHit;

    void SetupLog();
    void InitValues();
    void SetValue(RE::StaticFunctionTag* base, int index, int value);
    bool RegisterFuncs(RE::BSScript::IVirtualMachine* vm);
    void LoadGlobals();
}