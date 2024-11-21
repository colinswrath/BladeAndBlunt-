#pragma once

namespace BashBlockStaminaPatch
{
	float GetStaminaDamage(RE::HitData* a_hitData)
	{
		if (!a_hitData) {
			return 0.0f;
		}

		auto staggerMult = RE::GameSettingCollection::GetSingleton()->GetSetting("fStaminaBlockStaggerMult")->GetFloat();
		auto stamBlockDmgMult = RE::GameSettingCollection::GetSingleton()->GetSetting("fStaminaBlockDmgMult")->GetFloat();
		auto stamBlockBaseDmg = RE::GameSettingCollection::GetSingleton()->GetSetting("fStaminaBlockBase")->GetFloat();

		float perkMult = 1.0f;
		if (a_hitData->target) {
			auto actorPtr = a_hitData->target.get();

            auto leftHand = actorPtr->GetEquippedObject(true);       
			auto perk = Settings::GetSingleton()->BlockStaminaPerk;
            if (perk && leftHand && leftHand->IsArmor() && actorPtr->HasPerk(Settings::GetSingleton()->BlockStaminaPerk)) {
				perkMult = 0.5f;
			}
		}

		auto actualDmg = a_hitData->percentBlocked * a_hitData->physicalDamage;

		//NOTE: hitdata->stagger must be a float. Clib has it set to uint32_t which will mess things up. I changed it locally, but will need a PR to po3 clib
		auto stagger = a_hitData->stagger * staggerMult;
		auto damagScaleStam = actualDmg * stamBlockDmgMult;
		auto result = (stagger + stamBlockBaseDmg + damagScaleStam) * perkMult;

		return result;
	}

	float GetAttackStamina(RE::ActorValueOwner* a_avOwner, RE::BGSAttackData* a_attackData)
	{
		if (!a_attackData || !a_avOwner) {
			return 0.0f;
		}

		auto attackDataFlags = a_attackData->data.flags;
		auto powerAttack = attackDataFlags.all(RE::AttackData::AttackFlag::kPowerAttack);
		auto gameSettings = RE::GameSettingCollection::GetSingleton();

		RE::Actor* actor = &REL::RelocateMemberIfNewer<RE::Actor>(SKSE::RUNTIME_SSE_1_6_629, a_avOwner, -0xB0, -0xB8);

		if (a_attackData->data.flags.all(RE::AttackData::AttackFlag::kBashAttack)) {
			static auto* staminaPowerBashBase = gameSettings->GetSetting("fStaminaPowerBashBase");
			static auto* staminaBashBase = gameSettings->GetSetting("fStaminaBashBase");

			auto bashAttackStamina = powerAttack ? staminaPowerBashBase->GetFloat() : staminaBashBase->GetFloat();

			float playerBashPerkMult = 1.0f;
			if (actor && actor->IsPlayerRef()) {
                auto settings = Settings::GetSingleton();
                auto perk     = settings->BashStaminaPerk;
                auto perk25   = settings->BashStaminaPerk25;
				if (perk && actor->HasPerk(perk)) {
					playerBashPerkMult = 0.5f;
                } else if (perk25 && actor->HasPerk(perk25)) {
                    playerBashPerkMult = 0.75f;
                }
			}

			return (bashAttackStamina * a_attackData->data.staminaMult) * playerBashPerkMult;
		} else {
			if (!powerAttack) {
				return 0.0F;
			}

			auto* equippedObj = actor->GetEquippedObject(a_attackData->IsLeftAttack());
			auto equippedWeapon = equippedObj ? equippedObj->As<RE::TESObjectWEAP>() : nullptr;
			auto equippedWeaponWeight = equippedWeapon ? equippedWeapon->weight : 1.0F;

			if (!equippedWeapon) {
                equippedWeapon = Utility::GetUnarmedWeapon();
			}

			static auto* staminaAttackWeaponBase = gameSettings->GetSetting("fStaminaAttackWeaponBase");
			static auto* staminaAttackWeaponMultiplier = gameSettings->GetSetting("fStaminaAttackWeaponMult");
			static auto* powerAttackStaminaPenalty = gameSettings->GetSetting("fPowerAttackStaminaPenalty");

			auto powerAttackStamina = (staminaAttackWeaponBase->GetFloat() + (equippedWeaponWeight * staminaAttackWeaponMultiplier->GetFloat())) * powerAttackStaminaPenalty->GetFloat();

			RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINTS::kModPowerAttackStamina, actor, equippedWeapon, std::addressof(powerAttackStamina));

			return powerAttackStamina * a_attackData->data.staminaMult;
		}
	}

	bool InstallBlockMultHook()
	{
		struct stamDmg : Xbyak::CodeGenerator
		{
			stamDmg()
			{
				mov(rax, reinterpret_cast<std::uintptr_t>(GetStaminaDamage));
				jmp(rax);
			}
		};
        
		auto patch = new stamDmg();
		patch->ready();

		REL::safe_write(Hooks::GetStaminaDamageHook.address(), patch->getCode(), patch->getSize());
		return true;
	}

	//Bash hook will be installed kPostLoad due to a hook conflict with Scrambled Bugs.
	//Scrambled bugs left hand power attack fix has been absorbed into "GetAttackStamina"
	bool InstallBashMultHook()
	{
		struct stamDmg : Xbyak::CodeGenerator
		{
			stamDmg()
			{
				mov(rax, reinterpret_cast<std::uintptr_t>(GetAttackStamina));
				jmp(rax);
			}
		};

		auto patch = new stamDmg();
		patch->ready();

		REL::safe_write(Hooks::GetStaminaBashHook.address(), patch->getCode(), patch->getSize());
		return true;
	}
}
