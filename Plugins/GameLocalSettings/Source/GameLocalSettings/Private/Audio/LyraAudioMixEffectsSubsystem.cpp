// Copyright Epic Games, Inc. All Rights Reserved.


#include "Audio/LyraAudioMixEffectsSubsystem.h"

#include "AudioMixerBlueprintLibrary.h"
#include "AudioModulationStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "LoadingScreenManager.h"
#include "Audio/LyraAudioSettings.h"
#include "Settings/LyraSettingsLocal.h"
#include "Sound/SoundEffectSubmix.h"
#include "SoundControlBusMix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraAudioMixEffectsSubsystem)

class FSubsystemCollectionBase;

namespace
{
	template <class T>
	void LoadSoundBus(const FSoftObjectPath& MixPath, TObjectPtr<T> ControlBusMix, const FString& Info)
	{
		UObject* ObjPath = MixPath.TryLoad();
		if (!ObjPath) return;

		 auto SoundControlBusMix = Cast<T>(ObjPath);
		if (!SoundControlBusMix)
		{
			ensureMsgf(SoundControlBusMix, TEXT("%s reference missing from Lyra Audio Settings."), *Info);
			return;
		}

		ControlBusMix = SoundControlBusMix;
	};
}


void ULyraAudioMixEffectsSubsystem::Deinitialize()
{
	if (ULoadingScreenManager* LoadingScreenManager = UGameInstance::GetSubsystem<ULoadingScreenManager>(
		GetWorld()->GetGameInstance()))
	{
		LoadingScreenManager->OnLoadingScreenVisibilityChangedDelegate().RemoveAll(this);
		ApplyOrRemoveLoadingScreenMix(false);
	}

	Super::Deinitialize();
}

bool ULyraAudioMixEffectsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const bool bShouldCreateSubsystem = Super::ShouldCreateSubsystem(Outer);
	const UWorld* World = Outer->GetWorld();
	return !Outer || !World
		       ? bShouldCreateSubsystem
		       : DoesSupportWorldType(World->WorldType) && bShouldCreateSubsystem;
}

void ULyraAudioMixEffectsSubsystem::PostInitialize()
{
	if (const ULyraAudioSettings* LyraAudioSettings = GetDefault<ULyraAudioSettings>())
	{
		LoadSoundBus(LyraAudioSettings->DefaultControlBusMix, DefaultBaseMix, "Default Control Bus Mix");
		LoadSoundBus(LyraAudioSettings->LoadingScreenControlBusMix, LoadingScreenMix, "Loading Screen Control Bus Mix");
		LoadSoundBus(LyraAudioSettings->UserSettingsControlBusMix, UserMix, "User Control Bus Mix");
		LoadSoundBus(LyraAudioSettings->OverallVolumeControlBus, OverallControlBus, "Overall Control Bus");
		LoadSoundBus(LyraAudioSettings->MusicVolumeControlBus, MusicControlBus, "Music Control Bus");
		LoadSoundBus(LyraAudioSettings->SoundFXVolumeControlBus, SoundFXControlBus, "SoundFX Control Bus");
		LoadSoundBus(LyraAudioSettings->DialogueVolumeControlBus, DialogueControlBus, "Dialogue Control Bus");
		LoadSoundBus(LyraAudioSettings->VoiceChatVolumeControlBus, VoiceChatControlBus, "VoiceChat Control Bus");

		// Load HDR Submix Effect Chain
		HDRSubmixEffectChain = LoadSubmixEffectChain(LyraAudioSettings->HDRAudioSubmixEffectChain);
		// Load LDR Submix Effect Chain
		LDRSubmixEffectChain = LoadSubmixEffectChain(LyraAudioSettings->LDRAudioSubmixEffectChain);
	}

	ULoadingScreenManager* LoadingScreenManager = UGameInstance::GetSubsystem<ULoadingScreenManager>(
		GetWorld()->GetGameInstance());

	// Register with the loading screen manager
	if (!LoadingScreenManager) return;

	LoadingScreenManager->OnLoadingScreenVisibilityChangedDelegate().AddUObject(
		this, &ThisClass::OnLoadingScreenStatusChanged);
	ApplyOrRemoveLoadingScreenMix(LoadingScreenManager->GetLoadingScreenDisplayStatus());
}

TArray<FLyraAudioSubmixEffectsChain> ULyraAudioMixEffectsSubsystem::LoadSubmixEffectChain(
	const TArray<FLyraSubmixEffectChainMap>& SubmixEffectChainMap)
{
	TArray<FLyraAudioSubmixEffectsChain> SubmixEffectChain;
	for (const FLyraSubmixEffectChainMap& SoftSubmixEffectChain : SubmixEffectChainMap)
	{
		FLyraAudioSubmixEffectsChain NewEffectChain;
		UObject* SubmixObjPath = SoftSubmixEffectChain.Submix.LoadSynchronous();
		if (!SubmixObjPath) continue;

		USoundSubmix* SoundSubmix = Cast<USoundSubmix>(SubmixObjPath);
		if (!SoundSubmix) continue;

		NewEffectChain.Submix = SoundSubmix;
		TArray<USoundEffectSubmixPreset*> NewPresetChain;

		for (const TSoftObjectPtr<USoundEffectSubmixPreset>& SoftEffect : SoftSubmixEffectChain.SubmixEffectChain)
		{
			UObject* EffectObjPath = SoftEffect.LoadSynchronous();
			if (!EffectObjPath) continue;

			USoundEffectSubmixPreset* SubmixPreset = Cast<USoundEffectSubmixPreset>(EffectObjPath);
			if (!SubmixPreset) continue;

			NewPresetChain.Add(SubmixPreset);
		}

		NewEffectChain.SubmixEffectChain.Append(NewPresetChain);
		SubmixEffectChain.Add(NewEffectChain);
	}
	return SubmixEffectChain;
}

void ULyraAudioMixEffectsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	const UWorld* World = InWorld.GetWorld();
	if (!World) return;

	// Activate the default base mix
	if (DefaultBaseMix)
		UAudioModulationStatics::ActivateBusMix(World, DefaultBaseMix);

	// Retrieve the user settings
	const ULyraSettingsLocal* LyraSettingsLocal = GetDefault<ULyraSettingsLocal>();
	if (!LyraSettingsLocal) return;

	// Activate the User Mix
	if (UserMix)
	{
		UAudioModulationStatics::ActivateBusMix(World, UserMix);

		if (OverallControlBus && MusicControlBus && SoundFXControlBus && DialogueControlBus &&
			VoiceChatControlBus)
		{
			const auto OverallControlBusMixStage =
				UAudioModulationStatics::CreateBusMixStage(World,
				                                           OverallControlBus,
				                                           LyraSettingsLocal->GetOverallVolume());
			const auto MusicControlBusMixStage =
				UAudioModulationStatics::CreateBusMixStage(World,
				                                           MusicControlBus,
				                                           LyraSettingsLocal->GetMusicVolume());
			const auto SoundFXControlBusMixStage =
				UAudioModulationStatics::CreateBusMixStage(World,
				                                           SoundFXControlBus,
				                                           LyraSettingsLocal->GetSoundFXVolume());
			const auto DialogueControlBusMixStage =
				UAudioModulationStatics::CreateBusMixStage(World,
				                                           DialogueControlBus,
				                                           LyraSettingsLocal->GetDialogueVolume());
			const auto VoiceChatControlBusMixStage =
				UAudioModulationStatics::CreateBusMixStage(World,
				                                           VoiceChatControlBus,
				                                           LyraSettingsLocal->GetVoiceChatVolume());

			TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
			ControlBusMixStageArray.Add(OverallControlBusMixStage);
			ControlBusMixStageArray.Add(MusicControlBusMixStage);
			ControlBusMixStageArray.Add(SoundFXControlBusMixStage);
			ControlBusMixStageArray.Add(DialogueControlBusMixStage);
			ControlBusMixStageArray.Add(VoiceChatControlBusMixStage);

			UAudioModulationStatics::UpdateMix(World, UserMix, ControlBusMixStageArray);
		}
	}

	ApplyDynamicRangeEffectsChains(LyraSettingsLocal->IsHDRAudioModeEnabled());
}

void ULyraAudioMixEffectsSubsystem::ApplyDynamicRangeEffectsChains(const bool bHDRAudio) const
{
	TArray<FLyraAudioSubmixEffectsChain> AudioSubmixEffectsChainToApply;
	TArray<FLyraAudioSubmixEffectsChain> AudioSubmixEffectsChainToClear;

	// If HDR Audio is selected, then we clear out any existing LDR Submix Effect Chain Overrides
	// otherwise the reverse is the case.
	if (bHDRAudio)
	{
		AudioSubmixEffectsChainToApply.Append(HDRSubmixEffectChain);
		AudioSubmixEffectsChainToClear.Append(LDRSubmixEffectChain);
	}
	else
	{
		AudioSubmixEffectsChainToApply.Append(LDRSubmixEffectChain);
		AudioSubmixEffectsChainToClear.Append(HDRSubmixEffectChain);
	}

	// We want to collect just the submixes we need to actually clear,
	// otherwise they'll be overridden by the new settings
	TArray<USoundSubmix*> SubMixesLeftToClear;

	// We want to get the submixes that are not being overridden by the new effect chains,
	// so we can clear those out separately
	for (const FLyraAudioSubmixEffectsChain& EffectChainToClear : AudioSubmixEffectsChainToClear)
	{
		bool bAddToList = true;

		for (const FLyraAudioSubmixEffectsChain& SubmixEffectChain : AudioSubmixEffectsChainToApply)
		{
			if (SubmixEffectChain.Submix == EffectChainToClear.Submix)
			{
				bAddToList = false;
				break;
			}
		}

		if (bAddToList)
			SubMixesLeftToClear.Add(EffectChainToClear.Submix);
	}


	// Override submixes
	for (const auto& [Submix, SubmixEffectChain] : AudioSubmixEffectsChainToApply)
	{
		if (!Submix)continue;
		
		UAudioMixerBlueprintLibrary::SetSubmixEffectChainOverride(GetWorld(), Submix,
		                                                          SubmixEffectChain, 0.1f);
	}

	// Clear remaining submixes
	for (USoundSubmix* Submix : SubMixesLeftToClear)
	{
		UAudioMixerBlueprintLibrary::ClearSubmixEffectChainOverride(GetWorld(), Submix, 0.1f);
	}
}

void ULyraAudioMixEffectsSubsystem::OnLoadingScreenStatusChanged(const bool bShowingLoadingScreen)
{
	ApplyOrRemoveLoadingScreenMix(bShowingLoadingScreen);
}

void ULyraAudioMixEffectsSubsystem::ApplyOrRemoveLoadingScreenMix(const bool bWantsLoadingScreenMix)
{
	const UWorld* World = GetWorld();

	if (bAppliedLoadingScreenMix == bWantsLoadingScreenMix || !LoadingScreenMix || !World) return;

	// Apply the mix
	bWantsLoadingScreenMix
		? UAudioModulationStatics::ActivateBusMix(World, LoadingScreenMix)
		: UAudioModulationStatics::DeactivateBusMix(World, LoadingScreenMix);

	bAppliedLoadingScreenMix = bWantsLoadingScreenMix;
}

bool ULyraAudioMixEffectsSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// We only need this subsystem on Game worlds (PIE included)
	return (WorldType == EWorldType::Game || WorldType == EWorldType::PIE);
}
