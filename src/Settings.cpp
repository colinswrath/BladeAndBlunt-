#include "Settings.h"
#include <SimpleIni.h>
#include <sstream>
#include "Cache.h"

Settings* Settings::GetSingleton()
{
	static Settings settings;
	return &settings;
}

void Settings::LoadSettings()
{
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(R"(.\Data\SKSE\Plugins\BladeAndBlunt.ini)");

	enableInjuries = ini.GetBoolValue("", "bEnableInjuries", true);
	SMOnlyEnableInjuries = ini.GetBoolValue("", "bEnableInjuriesOnlyWithSM", false);
	enableSneakStaminaCost = ini.GetBoolValue("", "bEnableSneakStaminaCost", true);
	enableLevelDifficulty = ini.GetBoolValue("", "bLevelBasedDifficulty", true);
	zeroAllWeapStagger = ini.GetBoolValue("", "bZeroAllWeaponStagger", true);
	armorScalingEnabled = ini.GetBoolValue("", "bArmorRatingScalingEnabled", true);
	replaceAttackTypeKeywords = ini.GetBoolValue("", "bReplaceNewAttackTypeKeywords", true);

	std::string fileName(ini.GetValue("", "sModFileName", ""));
	FileName = fileName;
}


RE::FormID Settings::ParseFormID(const std::string& str)
{
	RE::FormID result;
	std::istringstream ss{ str };
	ss >> std::hex >> result;
	return result;
}

void Settings::AdjustWeaponStaggerVals()
{
	if (zeroAllWeapStagger) {
		logger::info("Adjusting weapon stagger values");
		int16_t totalWeaps = 0;

		auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (dataHandler) {
			for (const auto& foundWeap : dataHandler->GetFormArray<RE::TESObjectWEAP>()) {
				if (foundWeap && !foundWeap->weaponData.flags.any(RE::TESObjectWEAP::Data::Flag::kNonPlayable)) {
					foundWeap->weaponData.staggerValue = 0.0f;
					totalWeaps++;
				}
			}
		}

		logger::info(FMT_STRING("Stagger values adjusted: {} weapons"), totalWeaps);
	}
}

void Settings::ReplacePowerAttackKeywords()
{
	if (replaceAttackTypeKeywords) {

		logger::info("Swapping power attack types");
		std::unordered_map<std::string, RE::BGSKeyword*> swaps;

		//Hardcoded for now. This is all we need for b&b
		swaps["attackPowerStartH2HCombo"] = DualWieldReplaceKeyword;
		swaps["attackPowerStartDualWield"] = DualWieldReplaceKeyword;
		auto targetEvent = std::string("PowerAttackTypeStanding");

		auto dataHandler = RE::TESDataHandler::GetSingleton();
		auto races = dataHandler->GetFormArray<RE::TESRace>();

		for (auto raceCandidate : races) {
            if (raceCandidate->IsDeleted()) {
                continue;
            }

			auto& attackDataMap = raceCandidate->attackDataMap.get()->attackDataMap;

			for (auto& attackDataPtr : attackDataMap) {
				std::string eventName = std::string(attackDataPtr.first.c_str());

				auto eventIt = swaps.find(eventName);

				if (eventIt != swaps.end()) {
					auto attackType = attackDataPtr.second.get()->data.attackType;

					if (attackType) {
						auto attackTypeName = std::string(attackType->GetFormEditorID());
						auto raceName = std::string(raceCandidate->GetName());
						if (attackTypeName == targetEvent) {
							attackDataPtr.second.get()->data.attackType = eventIt->second;
							auto replacedName = std::string(DualWieldReplaceKeyword->GetFormEditorID());

                            logger::info("Updated race: {} - event: {} - {} -> {}", raceName, eventName, attackTypeName, replacedName);
						}
					}
				}
			}
		}

		logger::info("Power attack types swapped");
	}
}

void Settings::LoadForms()
{
	logger::info("Loading forms");
	auto dataHandler = RE::TESDataHandler::GetSingleton();
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\BladeAndBlunt.ini)");

    //BnB esp forms
    std::string attackingSpellFormID((ini.GetValue("", "IsAttackingSpellFormId", "")));
    IsAttackingSpell = LoadFormPointerFromIni<RE::SpellItem>(attackingSpellFormID, FileName);

    std::string blockingSpellFormID((ini.GetValue("", "IsBlockingSpellFormId", "")));
    IsBlockingSpell = LoadFormPointerFromIni<RE::SpellItem>(blockingSpellFormID, FileName);

    std::string sneakingSpellFormID((ini.GetValue("", "IsSneakingSpellFormId", "")));
    IsSneakingSpell = LoadFormPointerFromIni<RE::SpellItem>(sneakingSpellFormID, FileName);

    std::string sprintingSpellFormId((ini.GetValue("", "IsSprintingSpellFormId", "")));
    IsSprintingSpell = LoadFormPointerFromIni<RE::SpellItem>(sprintingSpellFormId, FileName);

    std::string mountSprintingSpellFormId((ini.GetValue("", "MountSprintingSpellFormId", "")));
    MountSprintingSpell = LoadFormPointerFromIni<RE::SpellItem>(mountSprintingSpellFormId, FileName);

    std::string bowStaminaSpellFormID((ini.GetValue("", "BowStaminaSpellFormId", "")));
    BowStaminaSpell = LoadFormPointerFromIni<RE::SpellItem>(bowStaminaSpellFormID, FileName);

    std::string xbowStaminaSpellFormID((ini.GetValue("", "XbowStaminaSpellFormId", "")));
    XbowStaminaSpell = LoadFormPointerFromIni<RE::SpellItem>(xbowStaminaSpellFormID, FileName);

    std::string IsCastingFormId((ini.GetValue("", "IsCastingSpellFormId", "")));
    IsCastingSpell = LoadFormPointerFromIni<RE::SpellItem>(IsCastingFormId, FileName);

    std::string injurySpell1FormID((ini.GetValue("", "InjurySpell1FormID", "")));
    InjurySpell1 = LoadFormPointerFromIni<RE::SpellItem>(injurySpell1FormID, FileName);

    std::string injurySpell2FormID((ini.GetValue("", "InjurySpell2FormID", "")));
    InjurySpell2 = LoadFormPointerFromIni<RE::SpellItem>(injurySpell2FormID, FileName);

    std::string injurySpell3FormID((ini.GetValue("", "InjurySpell3FormID", "")));
    InjurySpell3 = LoadFormPointerFromIni<RE::SpellItem>(injurySpell3FormID, FileName);

    std::string jumpSpellFormID((ini.GetValue("", "JumpSpellFormID", "")));
    jumpSpell = LoadFormPointerFromIni<RE::SpellItem>(jumpSpellFormID, FileName);

    std::string swimmingSpellFormID((ini.GetValue("", "SwimmingSpellFormId", "")));
    IsSwimmingSpell = LoadFormPointerFromIni<RE::SpellItem>(swimmingSpellFormID, FileName);

    std::string powerAttackStopSpellFormId((ini.GetValue("", "PowerAttackStopSpellFormId", "")));
    PowerAttackStopSpell = LoadFormPointerFromIni<RE::SpellItem>(powerAttackStopSpellFormId, FileName);

    //Injected Forms
    std::string injuryGlobal25FormId((ini.GetValue("", "InjuryChance25GlobalFormId", "")));
    InjuryChance25Health = LoadFormPointerFromIni<RE::TESGlobal>(injuryGlobal25FormId, "Update.esm");

    std::string injuryGlobal50FormId((ini.GetValue("", "InjuryChance50GlobalFormId", "")));
    InjuryChance50Health = LoadFormPointerFromIni<RE::TESGlobal>(injuryGlobal50FormId, "Update.esm");

    std::string injuryGlobal90FormId((ini.GetValue("", "InjuryChance90GlobalFormId", "")));
    InjuryChance90Health = LoadFormPointerFromIni<RE::TESGlobal>(injuryGlobal90FormId, "Update.esm");

    std::string bashPerkFormId((ini.GetValue("", "BashStaminaPerkFormId", "")));
    BashStaminaPerk = LoadFormPointerFromIni<RE::BGSPerk>(bashPerkFormId, "Update.esm");

    std::string bashPerk25FormId((ini.GetValue("", "BashStaminaPerk25FormId", "")));
    BashStaminaPerk25 = LoadFormPointerFromIni<RE::BGSPerk>(bashPerk25FormId, "Update.esm");
    
    std::string blockPerkFormId((ini.GetValue("", "BlockStaminaPerkFormId", "")));
    BlockStaminaPerk = LoadFormPointerFromIni<RE::BGSPerk>(blockPerkFormId, "Update.esm");

    std::string blockStaggerPerkFormId((ini.GetValue("", "BlockStaggerPerkFormId", "")));
    BlockStaggerPerk = LoadFormPointerFromIni<RE::BGSPerk>(blockStaggerPerkFormId, "Update.esm");

    std::string dualWieldKeywordFormId((ini.GetValue("", "DualWieldReplacementKeyword", "")));
    DualWieldReplaceKeyword = LoadFormPointerFromIni<RE::BGSKeyword>(dualWieldKeywordFormId, "Update.esm");

	//Hardcoded loads
	MAGParryControllerSpell = dataHandler->LookupForm(ParseFormID("0x817"), FileName)->As<RE::SpellItem>();

	MAGParryStaggerSpell = dataHandler->LookupForm(ParseFormID("0x816"), FileName)->As<RE::SpellItem>();
	MAGBlockStaggerSpell = dataHandler->LookupForm(ParseFormID("0x855"), FileName)->As<RE::SpellItem>();
	MAGBlockStaggerSpell2 = dataHandler->LookupForm(ParseFormID("0x858"), FileName)->As<RE::SpellItem>();

    MAG_ChargedShotSpell01 = dataHandler->LookupForm(ParseFormID("0x1B"), FileName)->As<RE::SpellItem>();
    MAG_ChargedShotSpell02 = dataHandler->LookupForm(ParseFormID("0x1E"), FileName)->As<RE::SpellItem>();
    MAG_ChargedShotSpell03 = dataHandler->LookupForm(ParseFormID("0x1F"), FileName)->As<RE::SpellItem>();

    MAG_ChargedShotTimer01 = dataHandler->LookupForm(ParseFormID("0x28"), FileName)->As<RE::TESGlobal>();
    MAG_ChargedShotTimer02 = dataHandler->LookupForm(ParseFormID("0x2C"), FileName)->As<RE::TESGlobal>();

	MAGCrossbowStaminaDrainSpell = dataHandler->LookupForm(ParseFormID("0x873"), FileName)->As<RE::SpellItem>();

	MAG_InjuryCooldown1 = dataHandler->LookupForm(ParseFormID("0x84F"), FileName)->As<RE::EffectSetting>();
	MAG_InjuryCooldown2 = dataHandler->LookupForm(ParseFormID("0x850"), FileName)->As<RE::EffectSetting>();

	MAG_ParryWindowEffect = dataHandler->LookupForm(ParseFormID("0x815"), FileName)->As<RE::EffectSetting>();
	MAG_InjuriesSMOnly = dataHandler->LookupForm(ParseFormID("0x88E"), FileName)->As<RE::TESGlobal>();

	MAG_levelBasedDifficulty = dataHandler->LookupForm(ParseFormID("0x854"), FileName)->As<RE::TESGlobal>();
	MAG_PowerAttackReplacement = dataHandler->LookupForm(ParseFormID("0xA9B"), FileName)->As<RE::TESGlobal>();
	MAG_InjuryAndRest = dataHandler->LookupForm(ParseFormID("0x83F"), FileName)->As<RE::TESGlobal>();
	HealthPenaltyUIGlobal = dataHandler->LookupForm(RE::FormID(0x2EDE), "Update.esm")->As<RE::TESGlobal>();

    MAG_DifficultyGlobal = dataHandler->LookupForm(RE::FormID(0xADA617), "Update.esm")->As<RE::TESGlobal>();

	auto smGlobal = dataHandler->LookupForm(RE::FormID(0x826), "ccqdrsse001-survivalmode.esl");

	if (smGlobal) {
		Survival_ModeEnabled = smGlobal->As<RE::TESGlobal>();
	}

	if (dataHandler->LookupModByName("Starfrost.esp"))
	{
		starfrostInstalled = true;
	}	

	SetGlobalsAndGameSettings();

	logger::info("Forms loaded");

}

void Settings::SetGlobalsAndGameSettings() 
{
	MAG_levelBasedDifficulty->value = enableLevelDifficulty;
	MAG_InjuryAndRest->value = enableInjuries;
	MAG_InjuriesSMOnly->value = SMOnlyEnableInjuries;
	MAG_PowerAttackReplacement->value = replaceAttackTypeKeywords;

	//Set fMaxArmorRating game setting
	auto gameSettings = RE::GameSettingCollection::GetSingleton();
	auto maxRatingSetting = gameSettings->GetSetting("fMaxArmorRating");

	if (armorScalingEnabled) {
		logger::info("Setting max armor rating to 90");
		maxRatingSetting->data.f = 90.0f;
	} else {
		logger::info("Setting max armor rating to 75");
		maxRatingSetting->data.f = 75.0f;
	}
}

