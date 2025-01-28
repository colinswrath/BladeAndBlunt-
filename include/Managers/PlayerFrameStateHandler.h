#pragma once

static float zoomTime{};

class PlayerFrameStateHandler
{
public:
    static void UpdatePlayerState(int frameCount);

private:
    static void HandleIsCastingState();
    static void HandleIsDrawingBowState();
    static void HandleIsDrawingXbowState();
    static void HandleIsAttackingState();
    static void HandleIsBlockingState();
    static void HandleIsSneakingState();
    static void HandleIsSwimmingState();
    static void HandleIsSprintingState();
    static void HandleIsBowZoomedState();
    static void RemoveAllStateSpells();
    static void ApplyZoomSpell();
    static void RemoveZoomSpells();
};
