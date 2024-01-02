#pragma once

#include "Settings.h"

namespace Hooks
{
	namespace
	{
		class PlayerCharacterEx :
			public RE::PlayerCharacter
		{
		public:
			void TryToSteal(RE::TESObjectREFR* a_fromRefr, RE::TESForm* a_item, RE::ExtraDataList* a_extraList)
			{
				if (!a_fromRefr || !currentProcess || !currentProcess->high) {
					return;
				}

				if (*Settings::useThreshold) {
					auto value = a_item->GetGoldValue();
					if (value > *Settings::costThreshold) {
						return;
					}
				}

				bool stolen = false;
				auto owner = a_fromRefr->GetOwner();
				if (!owner && a_fromRefr->Is(RE::FormType::ActorCharacter) && !a_fromRefr->IsDead(true)) {
					stolen = a_fromRefr != this;
				} else if (owner && owner != this) {
					stolen = true;
					if (owner->Is(RE::FormType::Faction)) {
						auto faction = static_cast<RE::TESFaction*>(owner);
						if (IsInFaction(faction)) {
							stolen = false;
						}
					}
				}

				if (stolen) {
					auto high = currentProcess->high;
					std::vector<RE::NiPointer<RE::Actor>> actors;
					{
						RE::BSReadLockGuard locker(high->knowledgeLock);
						for (auto& knowledgeData : high->knowledgeArray) {
							auto& knowledge = knowledgeData.second;
							if (knowledge) {
								auto akRef = knowledge->target.get();
								if (akRef) {
									actors.emplace_back(std::move(akRef));
								}
							}
						}
					}

					bool detected = false;
					for (auto& actor : actors) {
						if (!actor->IsPlayerTeammate() && !actor->IsDead(true) && actor->RequestDetectionLevel(this) > 0) {
							detected = true;
							break;
						}
					}

					if (!detected) {
						auto xOwnership = a_extraList->GetByType<RE::ExtraOwnership>();
						if (xOwnership) {
							xOwnership->owner = this;
						} else {
							xOwnership = new RE::ExtraOwnership(this);
							a_extraList->Add(xOwnership);
						}
					}
				}
			}

			void Hook_AddObjectToContainer(RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraList, std::int32_t a_count, RE::TESObjectREFR* a_fromRefr)
			{
				if (!a_extraList) {
					a_extraList = new RE::ExtraDataList();
				}

				TryToSteal(a_fromRefr, a_object, a_extraList);
				_AddObjectToContainer(this, a_object, a_extraList, a_count, a_fromRefr);
			}

			void Hook_PickUpObject(TESObjectREFR* a_item, std::uint32_t a_count, bool a_arg3, bool a_playSound)
			{
				TryToSteal(a_item, a_item->GetBaseObject(), &a_item->extraList);
				_PickUpObject(this, a_item, a_count, a_arg3, a_playSound);
			}

			static void InstallHooks()
			{
				REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };
				_AddObjectToContainer = vtbl.write_vfunc(0x5A, &Hook_AddObjectToContainer);
				_PickUpObject = vtbl.write_vfunc(0xCC, &Hook_PickUpObject);
			}

			static inline REL::Relocation<decltype(&Hook_AddObjectToContainer)> _AddObjectToContainer;
			static inline REL::Relocation<decltype(&Hook_PickUpObject)> _PickUpObject;
		};
	}

	void InstallHooks()
	{
		PlayerCharacterEx::InstallHooks();
	}
}
