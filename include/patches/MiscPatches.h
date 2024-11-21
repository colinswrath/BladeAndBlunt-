#pragma once

namespace MiscPatches
{
    class MiscPatches
    {

    public:

	    static bool InstallScalePatch();
	    static float GetScale(RE::TESObjectREFR* a1);
	    inline static REL::Relocation<decltype(GetScale)> _GetScaleFunction;
	    static bool InstallFBlockPatch();
	    static bool InstallSpellCapPatch();
        static bool InstallStaggerFormulaPatch();

	    static std::int32_t AbsorbCapPatch(RE::ActorValueOwner* akAvOwner, RE::ActorValue akValue);
    };
}
