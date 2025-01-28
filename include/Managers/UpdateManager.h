#pragma once

#include "Utility/Utility.h"
#include "Hooks.h"
#include "injury/InjuryPenaltyManager.h"
#include "Managers/TaskManager.h"
#include "Managers/PlayerFrameStateHandler.h"

using namespace Utility;

static int frameCount{};

class UpdateManager
{
public:
    static bool Install();

private:
    static std::int32_t OnFrameUpdate(std::int64_t a1);

	inline static REL::Relocation<decltype(OnFrameUpdate)> _OnFrameFunction;
};
