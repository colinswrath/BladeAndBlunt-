#include "UpdateManager.h"
#include "patches/ArmorRatingScaling.h"
#include "patches/BashBlockStaminaPatch.h"
#include "Events.h"
#include "patches/MiscPatches.h"

namespace Hooks
{
	bool InstallHooks()
	{
		if (!UpdateManager::Install()) { return false; }
		if (!MiscPatches::MiscPatches::InstallScalePatch()) { return false; }
		if (!MiscPatches::MiscPatches::InstallFBlockPatch()) { return false; }
		if (!MiscPatches::MiscPatches::InstallSpellCapPatch()) { return false; }
        if (!MiscPatches::MiscPatches::InstallStaggerFormulaPatch()) { return false; }

        AnimEventHandler::Install();
		WeaponFireHandler::InstallArrowReleaseHook();

		auto runtime = REL::Module::GetRuntime();
		if (Settings::GetSingleton()->armorScalingEnabled) {
			if (runtime == REL::Module::Runtime::AE) {
				logger::info("Installing ar hook AE");
				ArmorRatingScaling::InstallArmorRatingHookAE();
			} else {
				logger::info("Installing ar hook SE");
				ArmorRatingScaling::InstallArmorRatingHookSE();	
			}
			logger::info("Installed ar hook");
		}

		if (!BashBlockStaminaPatch::InstallBlockMultHook()) {
			return false;
		}

		return true;
	}

	bool InstallBashMultHook() 
	{
		return BashBlockStaminaPatch::InstallBashMultHook();
	}
}
