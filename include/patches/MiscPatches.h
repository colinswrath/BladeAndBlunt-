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

    class AnimEventHook
    {
    public:
        static void Install()
        {
            logger::info("Installing AnimEventHook hook");

            REL::Relocation<uintptr_t> AnimEventVtbl_PC{ RE::VTABLE_PlayerCharacter[2] };
            _ProcessEvent_PC  = AnimEventVtbl_PC.write_vfunc(0x1, ProcessEvent_PC);

        }

    private:
        static void ProcessCharacterEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                                              RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);
        static RE::BSEventNotifyControl ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                                        RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

        inline static REL::Relocation<decltype(ProcessEvent_PC)>  _ProcessEvent_PC;
        static void     HandleJumpAnim();


    };
}
