#include "Managers/PlayerFrameStateHandler.h"
#include "Settings.h"
#include "Utility/Utility.h"

void PlayerFrameStateHandler::UpdatePlayerState(int frameCount)
{
    RE::PlayerCharacter* player = Cache::GetPlayerSingleton();

    if (player->IsGodMode()) {
        RemoveAllStateSpells();
    }
    else {
        //Checks are offloaded to every X frame to avoid unnecessary work
        switch (frameCount) {
        case 1:
            HandleIsCastingState();
            break;
        case 2:
            HandleIsDrawingBowState();
            break;
        case 3:
            HandleIsDrawingXbowState();
            break;
        case 4:
            HandleIsAttackingState();
            HandleIsBlockingState();
            break;
        case 5:
            HandleIsSneakingState();
            HandleIsSwimmingState();
            break;
        case 6:
            HandleIsSprintingState();
            break;
        default:
            break;
        }
    }

    //Bow zoom state is handled every frame (that game is not paused)
    if (!Cache::GetUISingleton()->GameIsPaused()) {
        HandleIsBowZoomedState();
    }
}

void PlayerFrameStateHandler::HandleIsCastingState()
{
    auto settings = Settings::GetSingleton();
    RE::PlayerCharacter* player = Cache::GetPlayerSingleton();

    if (player->IsCasting(nullptr)) {
        if (settings->IsCastingSpell && !player->HasSpell(settings->IsCastingSpell)) {
            player->AddSpell(settings->IsCastingSpell);
        }
    }
    else if (settings->IsCastingSpell && player->HasSpell(settings->IsCastingSpell)) {
        player->RemoveSpell(settings->IsCastingSpell);
    }
}

void PlayerFrameStateHandler::HandleIsDrawingBowState()
{
    auto                 settings = Settings::GetSingleton();
    RE::PlayerCharacter* player   = Cache::GetPlayerSingleton();

    if (Utility::IsDrawingBow(player)) {

        if (!player->HasSpell(settings->BowStaminaSpell)) {
            player->AddSpell(settings->BowStaminaSpell);
        }
    }
    else if (player->HasSpell(settings->BowStaminaSpell)) {
        player->RemoveSpell(settings->BowStaminaSpell);
    }
}

void PlayerFrameStateHandler::HandleIsDrawingXbowState()
{
    const auto settings = Settings::GetSingleton();
    const auto player   = Cache::GetPlayerSingleton();

    const bool isDrawingXbow = Utility::IsDrawingBow(player, true);
    const bool hasXbowSpell  = player->HasSpell(settings->XbowStaminaSpell);

    if (isDrawingXbow && !hasXbowSpell) {
        player->AddSpell(settings->XbowStaminaSpell);
    }
    else if (!isDrawingXbow && hasXbowSpell) {
        player->RemoveSpell(settings->XbowStaminaSpell);
    }
}

void PlayerFrameStateHandler::HandleIsAttackingState()
{
    const auto settings = Settings::GetSingleton();
    const auto player   = Cache::GetPlayerSingleton();

    const bool isAttacking = player->IsAttacking();
    const bool hasAttackingSpell = player->HasSpell(settings->IsAttackingSpell);

    if (isAttacking) {
        settings->wasPowerAttacking = Utility::IsPowerAttacking(player);
        if (!hasAttackingSpell) {
            player->AddSpell(settings->IsAttackingSpell);
        }
    }
    else if (!isAttacking && hasAttackingSpell) {
        player->RemoveSpell(settings->IsAttackingSpell);
        if (settings->wasPowerAttacking) {
            settings->wasPowerAttacking = false;
            Utility::ApplySpell(player, player, settings->PowerAttackStopSpell);
        }
    }

}

void PlayerFrameStateHandler::HandleIsBlockingState()
{
    const auto settings = Settings::GetSingleton();
    const auto player   = Cache::GetPlayerSingleton();

    const bool isAttacking       = player->IsAttacking();

    if (isAttacking) {
        return;
    }

    const bool isBlocking       = player->IsBlocking();
    const bool hasBlockingSpell = player->HasSpell(settings->IsBlockingSpell);


    if (isBlocking) {
        // Parry setup
        if (!settings->IsBlockingWeaponSpellCasted) {
            settings->IsBlockingWeaponSpellCasted = true;
            Utility::ApplySpell(player, player, settings->MAGParryControllerSpell);
        }

        if (!hasBlockingSpell) {
            player->AddSpell(settings->IsBlockingSpell);
        }
    }
    else {
        settings->IsBlockingWeaponSpellCasted = false;
        if (hasBlockingSpell) {
            player->RemoveSpell(settings->IsBlockingSpell);
        }
    }
}

void PlayerFrameStateHandler::HandleIsSneakingState()
{
    const auto settings = Settings::GetSingleton();
    const auto player   = Cache::GetPlayerSingleton();

    const bool isSneakingAndMoving = player->IsSneaking() && player->IsMoving();
    const bool hasSneakingSpell    = player->HasSpell(settings->IsSneakingSpell);

    if (isSneakingAndMoving && !hasSneakingSpell && settings->enableSneakStaminaCost) {
        player->AddSpell(settings->IsSneakingSpell);
    }
    else if ((!isSneakingAndMoving || !settings->enableSneakStaminaCost) && hasSneakingSpell) {
        player->RemoveSpell(settings->IsSneakingSpell);
    }
}

void PlayerFrameStateHandler::HandleIsSwimmingState()
{
    const auto settings = Settings::GetSingleton();
    const auto player   = Cache::GetPlayerSingleton();

    const bool isSwimmingAndMoving = player->AsActorState()->IsSwimming() && player->IsMoving();
    const bool hasSwimmingSpell    = player->HasSpell(settings->IsSwimmingSpell);

    if (isSwimmingAndMoving && !hasSwimmingSpell) {
        player->AddSpell(settings->IsSwimmingSpell);
    }
    else if (!isSwimmingAndMoving && hasSwimmingSpell) {
        player->RemoveSpell(settings->IsSwimmingSpell);
    }
}

void PlayerFrameStateHandler::RemoveAllStateSpells()
{
    auto                 settings = Settings::GetSingleton();
    RE::PlayerCharacter* player   = Cache::GetPlayerSingleton();

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

void PlayerFrameStateHandler::HandleIsSprintingState()
{
    const auto   settings   = Settings::GetSingleton();
    const auto   player     = Cache::GetPlayerSingleton();

    RE::ActorPtr actorCheck = nullptr;
    bool         isMounted  = player->GetMount(actorCheck);
    auto         state      = isMounted ? actorCheck->AsActorState() : player->AsActorState();

    if (state->IsSprinting()) {
        if (isMounted) {
            actorCheck->AddSpell(settings->MountSprintingSpell);
        }
        else {
            if (!player->HasSpell(settings->IsSprintingSpell))
                player->AddSpell(settings->IsSprintingSpell);
        }
    }
    else if (player->HasSpell(settings->IsSprintingSpell)) {
        player->RemoveSpell(settings->IsSprintingSpell);

        if (actorCheck) {
            actorCheck->RemoveSpell(settings->MountSprintingSpell);
        }
    }
}

void PlayerFrameStateHandler::HandleIsBowZoomedState()
{
    if (Cache::g_deltaTime > 0) {
        auto playerCamera = RE::PlayerCamera::GetSingleton();

        if (playerCamera->bowZoomedIn) {
            // Note zoom start time
            zoomTime += Cache::g_deltaTime;
            // Apply appropriate spell based on time dif
            ApplyZoomSpell();
        }
        else {
            // Remove spells and reset zoom timer
            zoomTime = 0;
            RemoveZoomSpells();
        }
    }
}

void PlayerFrameStateHandler::ApplyZoomSpell()
{
    auto                 settings = Settings::GetSingleton();
    RE::PlayerCharacter* player   = Cache::GetPlayerSingleton();

    if (zoomTime > settings->MAG_ChargedShotTimer02->value) {
        player->RemoveSpell(settings->MAG_ChargedShotSpell02);
        player->AddSpell(settings->MAG_ChargedShotSpell03);
    }
    else if (zoomTime > settings->MAG_ChargedShotTimer01->value) {
        player->RemoveSpell(settings->MAG_ChargedShotSpell01);
        player->AddSpell(settings->MAG_ChargedShotSpell02);
    }
    else if (zoomTime < settings->MAG_ChargedShotTimer01->value) {
        player->AddSpell(settings->MAG_ChargedShotSpell01);
    }
}

void PlayerFrameStateHandler::RemoveZoomSpells()
{
    auto                 settings = Settings::GetSingleton();
    RE::PlayerCharacter* player   = Cache::GetPlayerSingleton();

    player->RemoveSpell(settings->MAG_ChargedShotSpell01);
    player->RemoveSpell(settings->MAG_ChargedShotSpell02);
    player->RemoveSpell(settings->MAG_ChargedShotSpell03);
}
