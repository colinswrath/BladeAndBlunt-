#pragma once

#include "Conditions.h"
#include "Hooks.h"
#include "InjuryPenaltyManager.h"

static float lastTime;

using namespace Conditions;

class UpdateManager
{
public:
	inline static int frameCount;
	inline static bool Install()
	{
		auto& trampoline = SKSE::GetTrampoline();
		_OnFrameFunction = trampoline.write_call<5>(Hooks::OnFrame_Update_Hook.address(), OnFrameUpdate);

		UpdateManager::frameCount = 0;
		logger::info("Installed hook for frame update");
		return true;
	}

private:
	inline static std::int32_t OnFrameUpdate(std::int64_t a1)
	{
		auto settings = Settings::GetSingleton();
            
		if (UpdateManager::frameCount > settings->maxFrameCheck) {
			UpdateManager::frameCount = 0;
		} else {
			RE::PlayerCharacter* player = Cache::GetPlayerSingleton();
			auto playerCamera = RE::PlayerCamera::GetSingleton();

            if (player->IsGodMode())
            {
                if (settings->IsCastingSpell)
                    player->RemoveSpell(settings->IsCastingSpell);

                if (settings->BowStaminaSpell)
                    player->RemoveSpell(settings->BowStaminaSpell);

                if (settings->IsAttackingSpell)
                    player->RemoveSpell(settings->IsAttackingSpell);

                if (settings->XbowStaminaSpell)
                    player->RemoveSpell(settings->XbowStaminaSpell);

                if (settings->IsAttackingSpell)
                    player->RemoveSpell(settings->IsAttackingSpell);

                if (settings->IsBlockingSpell)
                    player->RemoveSpell(settings->IsBlockingSpell);

                if (settings->IsSneakingSpell)
                    player->RemoveSpell(settings->IsSneakingSpell);

                if (settings->IsSprintingSpell)
                    player->RemoveSpell(settings->IsSprintingSpell);

                if (settings->IsSwimmingSpell)
                    player->RemoveSpell(settings->IsSwimmingSpell);
            }
            else {
			    switch (UpdateManager::frameCount) {
			    case 1:
				    if (player->IsCasting(nullptr)) {
					    if (settings->IsCastingSpell && !HasSpell(player, settings->IsCastingSpell)) {
						    player->AddSpell(settings->IsCastingSpell);
					    }
				    } else if (settings->IsCastingSpell && HasSpell(player, settings->IsCastingSpell)) {
					    player->RemoveSpell(settings->IsCastingSpell);
				    }
				    break;
			    case 2:
				    if (IsBowDrawNoZoomCheck(player, playerCamera)) {
					    if (!HasSpell(player, settings->BowStaminaSpell)) {
						    player->AddSpell(settings->BowStaminaSpell);
					    }
				    } else if (HasSpell(player, settings->BowStaminaSpell)) {
					    player->RemoveSpell(settings->BowStaminaSpell);
				    }
				    break;
			    case 3:
				    if (IsXbowDrawCheck(player, playerCamera)) {
					    if (!HasSpell(player, settings->XbowStaminaSpell)) {
						    player->AddSpell(settings->XbowStaminaSpell);
					    }
				    } else if (HasSpell(player, settings->XbowStaminaSpell)) {
					    player->RemoveSpell(settings->XbowStaminaSpell);
				    }
				    break;
			    case 4:
				    if (IsAttacking(player)) {
					    if (!HasSpell(player, settings->IsAttackingSpell)) {
						    player->AddSpell(settings->IsAttackingSpell);
					    }

                        settings->wasPowerAttacking = Conditions::IsPowerAttacking(player);

				    } else {
					    if (HasSpell(player, settings->IsAttackingSpell)) {
						    player->RemoveSpell(settings->IsAttackingSpell);
					    }

                        if (settings->wasPowerAttacking) {
                            settings->wasPowerAttacking = false;
                            Conditions::ApplySpell(player, player, settings->PowerAttackStopSpell);
                        }

					    if (IsBlocking(player)) {
						    auto leftHand = player->GetEquippedObject(true);
						    //Parry setup
						    if ((!leftHand || leftHand->IsWeapon()) && !settings->IsBlockingWeaponSpellCasted) {
							    settings->IsBlockingWeaponSpellCasted = true;
							    Conditions::ApplySpell(player, player, settings->MAGParryControllerSpell);
						    }

						    if (!HasSpell(player, settings->IsBlockingSpell)) {
							    player->AddSpell(settings->IsBlockingSpell);
						    }
					    } else {
						    if (HasSpell(player, settings->IsBlockingSpell)) {
							    player->RemoveSpell(settings->IsBlockingSpell);
						    }
						    settings->IsBlockingWeaponSpellCasted = false;
					    }
				    }
				    break;
			    case 5:
				    if (player->IsSneaking() && IsMoving(player)) {
					    if (!HasSpell(player, settings->IsSneakingSpell) && settings->enableSneakStaminaCost)
						    player->AddSpell(settings->IsSneakingSpell);
				    } else if (HasSpell(player, settings->IsSneakingSpell)) {
					    player->RemoveSpell(settings->IsSneakingSpell);
				    }

                    if (player->AsActorState()->IsSwimming() && IsMoving(player)) {
                        if (!HasSpell(player, settings->IsSwimmingSpell))
                            player->AddSpell(settings->IsSwimmingSpell);
                    }
                    else if (HasSpell(player, settings->IsSwimmingSpell)) {
                        player->RemoveSpell(settings->IsSwimmingSpell);
                    }

				    break;
			    case 6:
				    {
                        RE::ActorPtr actorCheck = nullptr;
                        bool isMounted = player->GetMount(actorCheck);
                        auto state = isMounted ? actorCheck->AsActorState() : player->AsActorState();

					    if (state->IsSprinting()) {
						    if (!HasSpell(player, settings->IsSprintingSpell))
							    player->AddSpell(settings->IsSprintingSpell);
                            if (isMounted) {
                                actorCheck->AddSpell(settings->MountSprintingSpell);
                            }

					    } else if (HasSpell(player, settings->IsSprintingSpell)) {
						    player->RemoveSpell(settings->IsSprintingSpell);

						    if (actorCheck) {
                                actorCheck->RemoveSpell(settings->MountSprintingSpell);
						    }
					    }
				    }
				    break;
			    default:
				    break;
			    }
            }
		}

        if (!Cache::GetUISingleton()->GameIsPaused()) {

		    if (Cache::g_deltaTime > 0) {
			    lastTime += Cache::g_deltaTime;
			    if (lastTime >= settings->injuryUpdateFrequency) {
				    auto inj = InjuryPenaltyHandler::GetSingleton();
				    inj->CheckInjuryAvPenalty();
				    lastTime = 0;
			    }
		    }
        }

 
		UpdateManager::frameCount++;
		return _OnFrameFunction(a1);
	}

	inline static REL::Relocation<decltype(OnFrameUpdate)> _OnFrameFunction;

	static bool IsXbowDrawCheck(RE::PlayerCharacter* player, RE::PlayerCamera* playerCamera)
	{
		auto attackState = player->AsActorState()->GetAttackState();

		if (playerCamera->bowZoomedIn) {
			return false;
		}

		auto equippedWeapon = skyrim_cast<RE::TESObjectWEAP*>(player->GetEquippedObject(false));
		if (!equippedWeapon) {
			return false;
		}

		switch (attackState) {
		case RE::ATTACK_STATE_ENUM::kBowDrawn:
			{
				if (equippedWeapon->GetWeaponType() == RE::WEAPON_TYPE::kCrossbow) {
					return true;
				}
				break;
			}
		default:
			{
				break;
			}
		}
		return false;
	}

	static bool IsBowDrawNoZoomCheck(RE::PlayerCharacter* player, RE::PlayerCamera* playerCamera)
	{
		auto attackState = player->AsActorState()->GetAttackState();

		if (playerCamera->bowZoomedIn) 
		{
			return false;
		}

		auto equippedWeapon = skyrim_cast<RE::TESObjectWEAP*>(player->GetEquippedObject(false));
		if (!equippedWeapon) {
			return false;
		}

		switch (attackState) {
		case RE::ATTACK_STATE_ENUM::kBowDrawn:
			{
				if (equippedWeapon->GetWeaponType() == RE::WEAPON_TYPE::kBow) {
					return true;
				}
				break;
			}
		case RE::ATTACK_STATE_ENUM::kBowAttached:
			{
				if (equippedWeapon->GetWeaponType() == RE::WEAPON_TYPE::kBow) {
					return true;
				}
				break;
			}
		default:
			{
				break;
			}
		}
		return false;
	}
};
