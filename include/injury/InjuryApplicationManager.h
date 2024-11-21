#pragma once

#include <RecentHitEventData.h>
#include <Utility.h>
#include <Settings.h>

class InjuryApplicationManager
{
public:
	std::multimap<std::uint32_t, RecentHitEventData> recentInjuryRolls;
	
	static InjuryApplicationManager* GetSingleton()
	{
		static InjuryApplicationManager appHandler;
		return &appHandler;
	}

	void ProcessHitInjuryApplication(RE::Actor* cause, RE::Actor* target, uint32_t runtime, float chanceMult)
	{
		auto settings = Settings::GetSingleton();
		if (((settings->enableInjuries && !settings->SMOnlyEnableInjuries) || (settings->enableInjuries && settings->SMOnlyEnableInjuries && Utility::IsSurvivalEnabled()))) {

			uint32_t roundedRunTime = RoundRunTime(runtime);
			if (!ShouldSkipInjuryRoll(cause, target, roundedRunTime)) {
                RollForInjuryEvent(chanceMult);
				recentInjuryRolls.insert(std::make_pair(roundedRunTime, RecentHitEventData(target, cause, roundedRunTime)));
			}
		}
	}

private:

	//this is separate from the "ShouldSkipHitEvent" because this is meant to track injury roll hits within the last second.
	//"ShouldSkipHitEvent" tracks duplicate events that happen at the same time (ie. hits from enchanted weapons trigger multiple hits)
	bool ShouldSkipInjuryRoll(RE::Actor* causeActor, RE::Actor* targetActor, std::uint32_t runTime)
	{
		bool skipEvent = false;

		auto secondRuntime = recentInjuryRolls.lower_bound(runTime - 1000);
		//Loop over all hits within the last 1 second of runtime
		for (auto it = secondRuntime; it != recentInjuryRolls.end(); ++it) {
			if (it->second.cause == causeActor && it->second.target == targetActor) {
				skipEvent = true;
				break;
			}
		}

		//Remove all events older than 1 second from runtime
		auto it = recentInjuryRolls.begin();
		while (it != secondRuntime) {
			it = recentInjuryRolls.erase(it);
		}

		return skipEvent;
	}

	static void RollForInjuryEvent(float chanceMult = 1.0f)
	{
		auto player = RE::PlayerCharacter::GetSingleton();
        auto maxHealth = Utility::GetMaxHealth();
		auto settings = Settings::GetSingleton();

        std::random_device rd;
        auto random = clib_util::RNG(rd()).Generate<float>(0.0f, 100.0f);
        auto* avOwner = player->AsActorValueOwner();
        auto health = avOwner->GetActorValue(RE::ActorValue::kHealth);
        auto injuryResist = avOwner->GetActorValue(RE::ActorValue::kShieldPerks);

        auto injuryResistMult = std::clamp((1 + injuryResist * -0.01f),0.25f,1.0f);
        auto finalChanceMult  = chanceMult * injuryResistMult;

        if (health <= maxHealth * 0.25f) {   //If health at or below 25%

			if (settings->InjuryChance25Health && random < settings->InjuryChance25Health->value * finalChanceMult) {
				ApplyInjury();
			}
		}
        else if (health <= maxHealth * 0.5f) {   //If health at or below 50%
            if (settings->InjuryChance50Health && random < settings->InjuryChance50Health->value * finalChanceMult) {
				ApplyInjury();
			}
        }
        else if (health <= maxHealth * 0.9f) {  //If health at or below 90%
            if (settings->InjuryChance90Health && random < settings->InjuryChance90Health->value * finalChanceMult) {
                ApplyInjury();
            }
        }
	}

	static void ApplyInjury()
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		auto settings = Settings::GetSingleton();

		if (player->HasSpell(settings->InjurySpell1)) {
            if (!Utility::PlayerHasActiveMagicEffect(settings->MAG_InjuryCooldown1)) {
				player->RemoveSpell(settings->InjurySpell1);
				player->AddSpell(settings->InjurySpell2);
			}
		} else if (player->HasSpell(settings->InjurySpell2)) {
            if (!Utility::PlayerHasActiveMagicEffect(settings->MAG_InjuryCooldown2)) {
				player->RemoveSpell(settings->InjurySpell2);
				player->AddSpell(settings->InjurySpell3);
			}
		} else if (!player->HasSpell(settings->InjurySpell3)) {
			player->AddSpell(settings->InjurySpell1);
		}
	}

	uint32_t RoundRunTime(uint32_t runTime)
	{
		auto flooredTime = static_cast<uint32_t>((runTime * 0.01f) + 0.5f);
		return flooredTime * 100;
	}
};
