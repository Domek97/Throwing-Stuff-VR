#pragma once
#include <SimpleIni.h>
#include "../Include/Functions.h"
#include <spdlog/sinks/basic_file_sink.h>

// Credit to Shizof for his Config.cpp in SpellWhellVR, which this implementation is based off of


namespace Config {
    CSimpleIniA ini;

    bool preventingHits = false;
    bool onlyPlayerHits = false;
    bool chainExplosions = false;
    bool potionChainExplosions = false;
    bool followersGetAngry = false;
    bool breakingIsCrime = false;
    bool throwingDispelsInvis = false;
    // bool onlyExplodeOnHit = false;

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

        // No longer used, but maybe will have need of ini in the future, so holding onto just in case
        //std::string configPath =
        //    std::filesystem::current_path().string() + "\\Data\\SKSE\\Plugins\\ThrowingStuffVR.ini";

        // Config::ini.SetUnicode();

        // SI_Error rc = Config::ini.LoadFile(configPath.c_str());
        // if (rc < 0) {
        //     logger::info("couldn't open INI. config path was: {}", configPath);
        // } else {
        //     logger::info("found INI. Loading values.");
        //     Config::preventingHits = Config::ini.GetBoolValue("Settings", "preventHits");
        //     Config::onlyPlayerHits = Config::ini.GetBoolValue("Settings", "onlyPlayerHits");
        //     Config::onlyPlayerHits = Config::ini.GetBoolValue("Settings", "onlyPlayerHits");
        //     Config::chainExplosions = Config::ini.GetBoolValue("Settings", "chainExplosions");
        //     Config::potionChainExplosions = Config::ini.GetBoolValue("Settings", "potionChainExplosions");
        //     Config::followersGetAngry = Config::ini.GetBoolValue("Settings", "followersGetAngry");
        //     Config::breakingIsCrime = Config::ini.GetBoolValue("Settings", "breakingIsCrime");
        //     Config::throwingDispelsInvis = Config::ini.GetBoolValue("Settings", "throwingDispelsInvis");
        //     // Functions::onlyExplodeOnHit = ini.GetBoolValue("Settings", "onlyExplodeOnHit");
        // }

    }

    void LoadGlobals() {
        preventingHits = RE::TESForm::LookupByEditorID<RE::TESGlobal>("_SOL_preventingHits")->value;
        onlyPlayerHits = RE::TESForm::LookupByEditorID<RE::TESGlobal>("_SOL_onlyPlayerHits")->value;
        chainExplosions = RE::TESForm::LookupByEditorID<RE::TESGlobal>("_SOL_chainExplosions")->value;
        potionChainExplosions = RE::TESForm::LookupByEditorID<RE::TESGlobal>("_SOL_potionChainExplosions")->value;
        followersGetAngry = RE::TESForm::LookupByEditorID<RE::TESGlobal>("_SOL_followersGetAngry")->value;
        breakingIsCrime = RE::TESForm::LookupByEditorID<RE::TESGlobal>("_SOL_breakingIsCrime")->value;
        throwingDispelsInvis = RE::TESForm::LookupByEditorID<RE::TESGlobal>("_SOL_breakingDispelsInvis")->value;
    }

    /// This function is used to set int config parameters from MCM to skse plugin.
    void SetValue(RE::StaticFunctionTag* base, int index, int value) {
        switch (index) { 
        case 1:
            preventingHits = value;
            break;
        case 2:
            onlyPlayerHits = value;
            break;
        case 3:
            chainExplosions = value;
            break;
        case 4:
            potionChainExplosions = value;
            break;
        case 5:
            followersGetAngry = value;
            break;
        case 6:
            breakingIsCrime = value;
            break;
        case 7:
            throwingDispelsInvis = value;
            break;
        }
    }

    bool RegisterFuncs(RE::BSScript::IVirtualMachine *vm) {
        vm->RegisterFunction("SetValue", "_SOL_NativeScript", SetValue);

        return true;
    }

}