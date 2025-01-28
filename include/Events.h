#pragma once
#include <Utility/Utility.h>
#include <injury/InjuryApplicationManager.h>
#include <RecentHitEventData.h>
#include "Managers/TaskManager.h"

using namespace Utility;

class OnHitEventHandler : public RE::BSTEventSink<RE::TESHitEvent>
{
public:
	std::multimap<std::uint32_t, RecentHitEventData> recentGeneralHits;

	static OnHitEventHandler* GetSingleton()
	{
		static OnHitEventHandler singleton;
		return &singleton;
	}

	//TODO-Temp fix until I can upgrade clib versions
	std::uint32_t GetDurationOfApplicationRunTime()
	{
		REL::Relocation<std::uint32_t*> runtime{ RELOCATION_ID(523662, 410201) };
		return *runtime;
	}

	RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override
	{
		if (!a_event || !a_event->target || !a_event->cause) {
			return RE::BSEventNotifyControl::kContinue;
		}

		auto causeActor = a_event->cause->As<RE::Actor>();
		auto targetActor = a_event->target->As<RE::Actor>();
	
		if (causeActor && targetActor && targetActor->IsPlayerRef() && !causeActor->IsPlayerRef()) {
			auto applicationRuntime = GetDurationOfApplicationRunTime();

			bool skipEvent = ShouldSkipHitEvent(causeActor, targetActor, applicationRuntime);	//Filters out dupe events

			if (!skipEvent) {			
				auto attackingWeapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(a_event->source);
				auto spellItem = RE::TESForm::LookupByID<RE::SpellItem>(a_event->source);

				//Something is effed with power attacks. The source isnt coming through and casting as a weapon and the hit flags are empty
				//We can work around it like this
				bool powerAttackMelee = false;
                if (a_event->flags.any(RE::TESHitEvent::Flag::kPowerAttack) || Utility::IsPowerAttacking(causeActor)) {
					bool rightIsMeleeWeapon = false;
					if (auto rightHandForm = causeActor->GetEquippedObject(false)) {
						if (rightHandForm->IsWeapon() && rightHandForm->As<RE::TESObjectWEAP>()->IsMelee()) {
							rightIsMeleeWeapon = true;
						}
					}

					bool LeftIsMeleeWeaponOrNone = false;
					auto leftHandForm = causeActor->GetEquippedObject(true);
					if (!leftHandForm || (leftHandForm->IsWeapon() && leftHandForm->As<RE::TESObjectWEAP>()->IsMelee()) || leftHandForm->IsArmor()) {
						LeftIsMeleeWeaponOrNone = true;
					}

					if (rightIsMeleeWeapon && LeftIsMeleeWeaponOrNone) {
						powerAttackMelee = true;
					}
				}

				bool isBlocking = a_event->flags.any(RE::TESHitEvent::Flag::kHitBlocked) || targetActor->IsBlocking();
                bool isWarding  = targetActor->HasKeywordString("MagicWard"sv);

				if ((attackingWeapon || powerAttackMelee) || (spellItem && spellItem->hostileCount > 0)) {

                    float chanceMult = isBlocking || isWarding ? 0.50f : 1.0f;

                    //Incoming spells while warding do not injure
                    if (!isWarding || !(spellItem && spellItem->hostileCount > 0)) {
					    auto injuryManager = InjuryApplicationManager::GetSingleton();
                        injuryManager->ProcessHitInjuryApplication(causeActor, targetActor, applicationRuntime, chanceMult);                   
                    }
				}

				bool blockedMeleeHit = false;
				if (!a_event->projectile && 
					((attackingWeapon && attackingWeapon->IsMelee()) || powerAttackMelee) &&
					isBlocking) {
					blockedMeleeHit = true;
				}

                if (blockedMeleeHit) {
				    //Stagger
					ProcessHitEventForBlockStagger(targetActor, causeActor);
					//Parry
					ProcessHitEventForParry(targetActor,causeActor);
				}
				recentGeneralHits.insert(std::make_pair(applicationRuntime, RecentHitEventData(targetActor, causeActor, applicationRuntime)));
			}
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	static void ProcessHitEventForParry(RE::Actor* target, RE::Actor* aggressor)
	{
		auto settings = Settings::GetSingleton();
        if (Utility::PlayerHasActiveMagicEffect(settings->MAG_ParryWindowEffect)) {
            Utility::ApplySpell(target, aggressor, settings->MAGParryStaggerSpell);
		}
	}

	static void ProcessHitEventForBlockStagger(RE::Actor* target, RE::Actor* aggressor)
	{
		auto settings = Settings::GetSingleton();
		if (target->HasPerk(settings->BlockStaggerPerk)) {
            Utility::ApplySpell(target, aggressor, settings->MAGBlockStaggerSpell2);
		}
		else {
			Utility::ApplySpell(target, aggressor, settings->MAGBlockStaggerSpell);
		}
	}

	bool ShouldSkipHitEvent(RE::Actor* causeActor, RE::Actor* targetActor, std::uint32_t runTime)
	{
		bool skipEvent = false;
		
		auto matchedHits = recentGeneralHits.equal_range(runTime);
		for (auto it = matchedHits.first; it != matchedHits.second; ++it) {
			if (it->second.cause == causeActor && it->second.target == targetActor) {
				skipEvent = true;
				break;
			}
		}

		auto upper = recentGeneralHits.lower_bound(runTime);
		auto it = recentGeneralHits.begin();
		while (it != upper) {
			it = recentGeneralHits.erase(it);
		}

		return skipEvent;
	}

	static void Register()
	{
		RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		eventHolder->AddEventSink(OnHitEventHandler::GetSingleton());
	}
};

class WeaponFireHandler
{
public:
	static void InstallArrowReleaseHook() {
		logger::info("Writing arrow release handler hook");

		auto& trampoline = SKSE::GetTrampoline();
		_Weapon_Fire = trampoline.write_call<5>(Hooks::arrow_release_handler.address(), WeaponFire);
		
		logger::info("Release arrow hooked");
	}

	static void WeaponFire(RE::TESObjectWEAP* a_weapon, RE::TESObjectREFR* a_source, RE::TESAmmo* a_ammo, RE::EnchantmentItem* a_ammoEnchantment, RE::AlchemyItem* a_poison)
	{
		_Weapon_Fire(a_weapon, a_source, a_ammo, a_ammoEnchantment, a_poison);

		if (!a_source) {
			return;
		}

		auto source = a_source->As<RE::Actor>();

		if (source->IsPlayerRef() && (a_weapon->IsCrossbow() || a_weapon->IsBow())) {
			Utility::ApplySpell(source, source,Settings::GetSingleton()->MAGCrossbowStaminaDrainSpell);
            RE::BSTSmartPointer<RE::BSAnimationGraphManager> manager;

            auto playerCamera = RE::PlayerCamera::GetSingleton();

            if (playerCamera->bowZoomedIn) {
                TaskManager::GetSingleton().AddTask(
                    [source]() {
                    source->NotifyAnimationGraph("attackStop");
                },
                std::chrono::milliseconds(500));
            }
		}

	}
	inline static REL::Relocation<decltype(WeaponFire)> _Weapon_Fire;
};

class AnimEventHandler
{
public:
    static void Install()
    {
        logger::info("Installing AnimEventHook hook");

        REL::Relocation<uintptr_t> AnimEventVtbl_PC{ RE::VTABLE_PlayerCharacter[2] };
        _ProcessEvent_PC = AnimEventVtbl_PC.write_vfunc(0x1, ProcessEvent_PC);
    }

private:


    inline static RE::BSEventNotifyControl ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                                            RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
    {
        ProcessCharacterEvent(a_sink, a_event, a_eventSource);
        return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
    }

    static void HandleJumpAnim()
    {
        auto settings = Settings::GetSingleton();
        auto player   = RE::PlayerCharacter::GetSingleton();
        if (!player->IsGodMode()) {
            Utility::ApplySpell(player, player, settings->jumpSpell);
        }
    }

    static void ProcessCharacterEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                              [[maybe_unused]] RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
    {
        if (!a_event->holder) {
            return;
        }

        std::string_view eventTag = a_event->tag.data();
        uint32_t         str      = hash(eventTag.data(), eventTag.size());

        RE::Actor* actor = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());
        if (!actor) {
            return;
        }

        if (str == "JumpUp"_h) {
            HandleJumpAnim();
        }
    }
    inline static REL::Relocation<decltype(ProcessEvent_PC)> _ProcessEvent_PC;
};

