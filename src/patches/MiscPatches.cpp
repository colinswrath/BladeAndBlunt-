#include "patches/MiscPatches.h"
#include "Hooks.h"
#include "Settings.h"
#include "Utility/Cache.h"
#include "Utility/Utility.h"

namespace MiscPatches
{
    bool MiscPatches::InstallScalePatch() 
    {
	    auto& trampoline = SKSE::GetTrampoline();
	    _GetScaleFunction = trampoline.write_call<5>(Hooks::Scale_Patch_Hook.address(), GetScale);

	    logger::info("Installed hook for scale patch");
	    return true;
    }

    float MiscPatches::GetScale(RE::TESObjectREFR* a1)
    {
	    auto scale = _GetScaleFunction(a1);
	    if (skyrim_cast<RE::Actor*>(a1) == RE::PlayerCharacter::GetSingleton()) {
		    return 1.0f;
	    } else {
		    return scale;
	    }
    }

    bool MiscPatches::InstallFBlockPatch()
    {
	    const std::int32_t fBlockOffset = static_cast<std::int32_t>(Hooks::fBlock_GameSetting.address() - (Hooks::Block_GameSetting_Hook.address() + 0x8));

	    REL::safe_write(Hooks::Block_GameSetting_Hook.address() + 0x4, fBlockOffset);

	    logger::info("Block max hook installed");
	    return true;
    }

    bool MiscPatches::InstallSpellCapPatch()
    {
	    const std::uint8_t expectedBytes[] = { 0x41, 0xFF, 0x50, 0x08, 0xF3, 0x0F, 0x2C, 0xC0 };
	    std::uint8_t noopPatch[] = { 0x90, 0x90, 0x90 };

	    if (std::memcmp(reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(Hooks::SpellCap_Hook.address())), expectedBytes, sizeof(expectedBytes))) {
		    logger::error("ERROR: Expected byte mismatch on Spell Absorb Cap hook");
		    return false;
	    }

	    auto& trampoline = SKSE::GetTrampoline();
	    trampoline.write_call<5>(Hooks::SpellCap_Hook.address(), AbsorbCapPatch);
	    REL::safe_write(Hooks::SpellCap_Hook.address() + 0x5, noopPatch, 3);

	    logger::info("Absorb cap hook installed.");
	    return true;
    }

    std::int32_t MiscPatches::AbsorbCapPatch(RE::ActorValueOwner* akAvOwner, RE::ActorValue akValue)
    {
	    auto cap = (int32_t)akAvOwner->GetActorValue(akValue);
	    float playerMax = Cache::GetfPlayerMaxResistSingleton()->GetFloat();
	    auto max = (int32_t)playerMax;

	    if (cap > playerMax) {
		    return max;
	    } else {
		    return cap;
	    }
    }

    bool MiscPatches::InstallStaggerFormulaPatch()
    {
        logger::info("Installing Stagger patch");

        const std::uint8_t expectedBytes[] = { 0xE8 };
        std::uint8_t staggerPatch[] = { 0x0F, 0x28, 0xC3, 0x66, 0x90 };

        if (std::memcmp(reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(Hooks::staggerHook.address())), expectedBytes, sizeof(expectedBytes))) {
            logger::error("ERROR: Expected byte mismatch on stagger patch hook");
            return false;
        }

        REL::safe_write(Hooks::staggerHook.address(), staggerPatch, 5);

	    logger::info("Stagger patch installed");
        return true;
    }
}
