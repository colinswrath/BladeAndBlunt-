#pragma once

#include "Cache.h"
#include "Utility.h"

class InjuryPenaltyHandler
{
public:
	static InjuryPenaltyHandler* GetSingleton()
	{
		static InjuryPenaltyHandler penaltyHandler;
		return &penaltyHandler;
	}

	float currentInjuryPenalty;

	void CheckInjuryAvPenalty()
	{
		auto settings = Settings::GetSingleton();
		if ((settings->enableInjuries && !settings->SMOnlyEnableInjuries) || 
			(settings->SMOnlyEnableInjuries && settings->enableInjuries && Utility::IsSurvivalEnabled())) {
			auto player = Cache::GetPlayerSingleton();

			if (player->HasSpell(settings->InjurySpell1)) {
				ApplyAttributePenalty(settings->injury1AVPercent);
			} else if (player->HasSpell(settings->InjurySpell2)) {
				ApplyAttributePenalty(settings->injury2AVPercent);
			} else if (player->HasSpell(settings->InjurySpell3)) {
				ApplyAttributePenalty(settings->injury3AVPercent);
			} else {
				RemoveAttributePenalty();
			}
		} else {
			RemoveAttributePenalty();
		}
	}

	void ApplyAttributePenalty(float percentPen)
	{
		auto player = RE::PlayerCharacter::GetSingleton();

		float maxPenAv = GetMaxHealthAv();

		float lastPenaltyMag = currentInjuryPenalty;
		float newPenaltyMag = std::roundf(maxPenAv * percentPen);

		if (newPenaltyMag > maxPenAv) {
			newPenaltyMag = maxPenAv;
		}
		auto magDelta = lastPenaltyMag - newPenaltyMag;

		if (magDelta < 0) {
			player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -1*magDelta);  //Damage or restore AV
		}

		//Set tracker av not actual damage
		currentInjuryPenalty = newPenaltyMag;
		
		player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth, magDelta);	//Damage or restore AV

		SetAttributePenaltyUIGlobal(percentPen);
	}

	void RemoveAttributePenalty()
	{
		auto player = RE::PlayerCharacter::GetSingleton();

		float currentPenaltyMag = currentInjuryPenalty;

		if (currentPenaltyMag > 0) {
			currentInjuryPenalty = 0.0f;
			SetAttributePenaltyUIGlobal(0.0f);
			player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth, currentPenaltyMag);
		}
	}

	static void SetAttributePenaltyUIGlobal(float penaltyPerc)
	{
		auto newVal = penaltyPerc * 100.0f;
		newVal = std::clamp(newVal, 0.0f, 100.0f);
		auto settings = Settings::GetSingleton();
		if (settings->HealthPenaltyUIGlobal) {
			settings->HealthPenaltyUIGlobal->value = newVal;
		}
	}

	float GetMaxHealthAv()
	{
		return Utility::GetMaxHealth() + currentInjuryPenalty;
	}
};
