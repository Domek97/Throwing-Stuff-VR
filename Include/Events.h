#pragma once
#include "../Include/Functions.h"
#include "../Include/BombHandler.h"
#include "../Source/RE/TESDestructionStageChangedEvent.h"
#include "../Include/Config.h"

class ObjectEventSink : public RE::BSTEventSink<RE::TESHitEvent>,
                        public RE::BSTEventSink<RE::TESDestructionStageChangedEvent> {
    
    ObjectEventSink() = default;

    ObjectEventSink(const ObjectEventSink&) = delete;

    ObjectEventSink(ObjectEventSink&&) = delete;

    ObjectEventSink& operator=(const ObjectEventSink&) = delete;

    ObjectEventSink& operator=(ObjectEventSink&&) = delete;


    public:
        static ObjectEventSink* GetSingleton() {
            static ObjectEventSink singleton;
            return std::addressof(singleton);
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*);

        RE::BSEventNotifyControl ProcessEvent(const RE::TESDestructionStageChangedEvent* event, RE::BSTEventSource<RE::TESDestructionStageChangedEvent>*);
};