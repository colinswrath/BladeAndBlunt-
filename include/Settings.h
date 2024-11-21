#pragma once

class Settings
{
public:
	static Settings* GetSingleton();

	void LoadSettings();
	void LoadForms();
	void AdjustWeaponStaggerVals();
	void ReplacePowerAttackKeywords();
	void SetGlobalsAndGameSettings();

	RE::SpellItem* IsAttackingSpell;
	RE::SpellItem* IsBlockingSpell;
	RE::SpellItem* IsSneakingSpell;
    RE::SpellItem* IsSprintingSpell;
    RE::SpellItem* IsSwimmingSpell;
	RE::SpellItem* MountSprintingSpell;
	RE::SpellItem* BowStaminaSpell;
	RE::SpellItem* XbowStaminaSpell;
	RE::SpellItem* IsCastingSpell;
	RE::SpellItem* MAGParryStaggerSpell;
	RE::SpellItem* MAGBlockStaggerSpell;
	RE::SpellItem* MAGBlockStaggerSpell2;
	RE::SpellItem* MAGParryControllerSpell;
	RE::SpellItem* MAGCrossbowStaminaDrainSpell;

	RE::SpellItem* InjurySpell1;
	RE::SpellItem* InjurySpell2;
    RE::SpellItem* InjurySpell3;
    RE::SpellItem* jumpSpell;
    RE::SpellItem* PowerAttackStopSpell;

    RE::SpellItem* MAG_ChargedShotSpell01;
    RE::SpellItem* MAG_ChargedShotSpell02;
    RE::SpellItem* MAG_ChargedShotSpell03;
	
	RE::BGSPerk* BashStaminaPerk;
    RE::BGSPerk* BashStaminaPerk25;
	RE::BGSPerk* BlockStaminaPerk;
	RE::BGSPerk* BlockStaggerPerk;

	RE::TESGlobal* InjuryChance90Health;
    RE::TESGlobal* InjuryChance50Health;
	RE::TESGlobal* InjuryChance25Health;
	RE::TESGlobal* Survival_ModeEnabled;
    RE::TESGlobal* MAG_InjuriesSMOnly;
    RE::TESGlobal* MAG_DifficultyGlobal;

    RE::TESGlobal* MAG_ChargedShotTimer01;
    RE::TESGlobal* MAG_ChargedShotTimer02;

	RE::TESGlobal* MAG_levelBasedDifficulty;
	RE::TESGlobal* MAG_InjuryAndRest;
	RE::TESGlobal* HealthPenaltyUIGlobal;
	RE::TESGlobal* MAG_PowerAttackReplacement;

	RE::EffectSetting* MAG_ParryWindowEffect;
	RE::EffectSetting* MAG_InjuryCooldown1;
	RE::EffectSetting* MAG_InjuryCooldown2;

    RE::StaggerEffect* stagger;

	RE::BGSKeyword* DualWieldReplaceKeyword;

	bool enableInjuries;
	bool SMOnlyEnableInjuries;
	bool enableSneakStaminaCost;
	bool enableLevelDifficulty;
	bool replaceAttackTypeKeywords;
	bool zeroAllWeapStagger;
	bool armorScalingEnabled;
	bool starfrostInstalled;

    bool wasPowerAttacking=false;
	
	float injury1AVPercent;
	float injury2AVPercent;
	float injury3AVPercent;
	float injuryUpdateFrequency = 0.5f;
    float zoomTimeThreshold2    = 3.0;
    float zoomTimeThreshold3    = 10.0; 

	bool IsBlockingWeaponSpellCasted = false;

	int maxFrameCheck = 7;

	static RE::FormID ParseFormID(const std::string& str);

	std::string FileName;

	static RE::TESFile* LookupLoadedModByName(std::string_view a_modName)
    {
        for (auto& file : RE::TESDataHandler::GetSingleton()->compiledFileCollection.files) {
            if (a_modName.size() == strlen(file->fileName) && _strnicmp(file->fileName, a_modName.data(), a_modName.size()) == 0) {
                return file;
            }
        }
        return nullptr;
    }

    static const RE::TESFile* LookupLoadedLightModByName(std::string_view a_modName)
    {
        for (auto& smallFile : RE::TESDataHandler::GetSingleton()->compiledFileCollection.smallFiles) {
            if (a_modName.size() == strlen(smallFile->fileName) && _strnicmp(smallFile->fileName, a_modName.data(), a_modName.size()) == 0) {
                return smallFile;
            }
        }
        return nullptr;
    }

    template<typename T>
    T* LoadFormPointerFromIni(std::string formId, std::string filename)
    {
        if (!formId.empty()) {
            auto objectFormId = ParseFormID(formId);
            auto form         = RE::TESDataHandler::GetSingleton()->LookupForm(objectFormId, filename)->As<T>();
            return form;
        }
        else {
            return nullptr;
        }
        
    }
};
