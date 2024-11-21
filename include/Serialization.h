#pragma once
#include "injury/InjuryPenaltyManager.h"

namespace Serialization
{
	static constexpr std::uint32_t SerializationVersion = 1;
	static constexpr std::uint32_t ID = 'BBLT';
	static constexpr std::uint32_t SerializationType = 'BBLR';


	inline void SaveCallback(SKSE::SerializationInterface* a_skse)
	{

		if (!a_skse->OpenRecord(SerializationType, SerializationVersion)) {
			logger::error("Failed to open blade and blunt record");
			return;
		} else {
			auto injManager = InjuryPenaltyHandler::GetSingleton();
			auto penToSerialize = injManager->currentInjuryPenalty;

			if (!a_skse->WriteRecordData(penToSerialize)) {
				logger::error("Failed to write size of record data");
				return;
			}else { 
				logger::info(FMT_STRING("Serialized: {}"), std::to_string(penToSerialize));
			}
		}
	}

	inline void LoadCallback(SKSE::SerializationInterface* a_skse)
	{
		std::uint32_t type;
		std::uint32_t version;
		std::uint32_t length;
		a_skse->GetNextRecordInfo(type, version, length);

		auto injManager = InjuryPenaltyHandler::GetSingleton();

		if (type != SerializationType) {
			return;
		}

		if (version != SerializationVersion) {
			logger::error("Unable to load data");
			return;
		}

		float deserializedVal;
		if (!a_skse->ReadRecordData(deserializedVal)) {
			logger::error("Failed to load size");
			return;
		} else {
            /*injManager->currentInjuryPenalty = deserializedVal;
            logger::info(FMT_STRING("Deserialized: {}"), std::to_string(deserializedVal));*/
			injManager->currentInjuryPenalty = 0;
            injManager->RemoveAttributePenalty();
            logger::info("Existing save info found. Removing attribute penalty.");
		}
	}

	inline void RevertCallback([[maybe_unused]] SKSE::SerializationInterface* a_skse)
	{
		auto injManager = InjuryPenaltyHandler::GetSingleton();
		injManager->currentInjuryPenalty = 0.0f;	
	}
}
