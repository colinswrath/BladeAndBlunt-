#include "Managers/UpdateManager.h"


std::int32_t UpdateManager::OnFrameUpdate(std::int64_t a1)
{
    auto settings = Settings::GetSingleton();

    if (frameCount > settings->maxFrameCheck) {
        frameCount = 0;
    }
    else {
        PlayerFrameStateHandler::UpdatePlayerState(frameCount);

        // Check difficulty global
        settings->MAG_DifficultyGlobal->value = Cache::GetPlayerSingleton()->GetGameStatsData().difficulty;
    }

    if (!Cache::GetUISingleton()->GameIsPaused()) {
        // Check for tasks to perform. ie. delaying forcing out of bow draw use the task system
        TaskManager::GetSingleton().Update();
    }

    frameCount++;
    return _OnFrameFunction(a1);
}

bool UpdateManager::Install()
{
    auto& trampoline = SKSE::GetTrampoline();
    _OnFrameFunction = trampoline.write_call<5>(Hooks::OnFrame_Update_Hook.address(), OnFrameUpdate);

    frameCount = 0;
    logger::info("Installed hook for frame update");
    return true;
}
