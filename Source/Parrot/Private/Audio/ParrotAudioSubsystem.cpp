// 2024 Secret Dimension, Inc.


#include "Audio/ParrotAudioSubsystem.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Engine/WorldInitializationValues.h"
#include "Game/ParrotGameInstance.h"
#include "Game/ParrotGameState.h"
#include "Kismet/GameplayStatics.h"
#include "LoadingScreenManager.h"
#include "PlatformFeatures.h"
#include "Settings/ParrotDeveloperSettings.h"
#include "Settings/ParrotGameUserSettings.h"
#include "Settings/ParrotWorldSettings.h"

DEFINE_LOG_CATEGORY(LogParrotAudioSubsystem); 

void UParrotAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection); 

	// Ask the loading screen to notify us when its visibility changes
	TObjectPtr<ULoadingScreenManager> LoadingScreenManager = Collection.InitializeDependency<ULoadingScreenManager>();
	LoadingScreenManager->OnLoadingScreenVisibilityChangedDelegate().AddUObject(this, &UParrotAudioSubsystem::LoadingScreenVisibilityChanged);

	// Get a reference to our game instance 
	ParrotGameInstance = Cast<UParrotGameInstance>(GetGameInstance()); 

	// Get the class default object for Parrot Developer Settings 
	CurrentDeveloperSettings = GetDefault<UParrotDeveloperSettings>();

	// Load default audio classes from the Parrot Developer Project Settings 
	// Note that we're using IsNull here to check if the soft object pointer path is null
	// IsValid, checks an already loaded object 

	// Default Sound Mixer
	if (!CurrentDeveloperSettings->DefaultSoundMixModifier.IsNull())
	{
		DefaultSoundMixModifier = CurrentDeveloperSettings->DefaultSoundMixModifier.LoadSynchronous(); 
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::Initialize DefaultSoundMixModifier is invalid, check your Parrot Developer settings."));
	}

	// Main Sound 
	if (!CurrentDeveloperSettings->MainSoundClass.IsNull())
	{
		MainSoundClass = CurrentDeveloperSettings->MainSoundClass.LoadSynchronous();
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::Initialize MainSoundClass is invalid, check your Parrot Developer settings."));
	}

	// Music 
	if (!CurrentDeveloperSettings->MusicSoundClass.IsNull())
	{
		MusicSoundClass = CurrentDeveloperSettings->MusicSoundClass.LoadSynchronous();
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::Initialize MusicSoundClass is invalid, check your Parrot Developer settings."));
	}

	// SFX 
	if (!CurrentDeveloperSettings->SFXSoundClass.IsNull())
	{
		SFXSoundClass = CurrentDeveloperSettings->SFXSoundClass.LoadSynchronous();
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::Initialize SFXSoundClass is invalid, check your Parrot Developer settings."));
	}

	// Load the (optional) default music for the game from the Parrot Developer Project Settings 
	if (!CurrentDeveloperSettings->LevelCompleteMusic.IsNull())
	{
		LevelCompleteMusic = CurrentDeveloperSettings->LevelCompleteMusic.LoadSynchronous(); 
	}

	if (!CurrentDeveloperSettings->LevelFailMusic.IsNull())
	{
		LevelFailMusic = CurrentDeveloperSettings->LevelFailMusic.LoadSynchronous(); 
	}

	// Bind to world initialization so that the audio subsystem knows about the world state (BeginPlay) 
	PostWorldCreationHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UParrotAudioSubsystem::WorldInitialization);
}

void UParrotAudioSubsystem::SaveAudioSettings()
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->SaveSettings();
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::SaveAudioSettings CurrentGameSettings is nullptr."));
		return;
	}
}

void UParrotAudioSubsystem::SetMainVolume(float NewVolume, float FadeIn)
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->MainVolume = NewVolume;

		ApplyVolumeChangeToMix(MainSoundClass, CurrentGameSettings->MainVolume, FadeIn);
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::SetMainVolume CurrentGameSettings is nullptr."));
		return;
	}
}

void UParrotAudioSubsystem::SetMusicVolume(float NewVolume, float FadeIn)
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->MusicVolume = NewVolume;

		ApplyVolumeChangeToMix(MusicSoundClass, CurrentGameSettings->MusicVolume, FadeIn);
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::SetMusicVolume CurrentGameSettings is nullptr."));
		return;
	}
}

void UParrotAudioSubsystem::SetSFXVolume(float NewVolume, float FadeIn)
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->SFXVolume = NewVolume;

		ApplyVolumeChangeToMix(SFXSoundClass, CurrentGameSettings->SFXVolume, FadeIn);
	}
	else
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::SetSFXVolume CurrentGameSettings is nullptr."));
		return;
	}
}

float UParrotAudioSubsystem::GetMainVolume()
{
	if (CurrentGameSettings)
	{
		return CurrentGameSettings->MainVolume;
	}
	
	UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::GetMainVolume CurrentGameSettings is nullptr."));
	return 1.0f; 
}

float UParrotAudioSubsystem::GetMusicVolume()
{
	if (CurrentGameSettings)
	{
		return CurrentGameSettings->MusicVolume;
	}

	UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::GetMusicVolume CurrentGameSettings is nullptr."));
	return 1.0f;
}

float UParrotAudioSubsystem::GetSFXVolume()
{
	if (CurrentGameSettings)
	{
		return CurrentGameSettings->SFXVolume;
	}

	UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::GetSFXVolume CurrentGameSettings is nullptr."));
	return 1.0f;
}

bool UParrotAudioSubsystem::WorldMusicInitialized()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return false; 
	}

	return true;
}

void UParrotAudioSubsystem::PlaySoundAsWorldMusic(USoundBase* Music)
{
	PlayWorldMusic(Music); 
}

void UParrotAudioSubsystem::ApplyVolumeChangeToMix(USoundClass* TargetSoundClass, float Volume, float FadeIn)
{
	// Get the audio device, apply the override to the mix, push modifier update 
	if (FAudioDeviceHandle AudioDevice = CurrentWorld->GetAudioDevice())
	{
		AudioDevice->SetSoundMixClassOverride(
			DefaultSoundMixModifier, /* Sound Mix Modifier */
			TargetSoundClass, /* Sound Class */
			Volume, /* Volume Multiplier*/
			1.0f, /* Pitch Multiplier */
			FadeIn, /* Fade In Time */
			true /* Apply To Children */
		);
		AudioDevice->PushSoundMixModifier(DefaultSoundMixModifier);
	}
}

void UParrotAudioSubsystem::WorldInitialization(UWorld* World, const FWorldInitializationValues IVS)
{
	if (World)
	{
		// The world has been initialized so now we can bind to BeginPlay of the world. 
		// Here, we bind our world begin play function to this delegate. 
		CurrentWorld = World; 
		OnWorldBeginPlayHandle = CurrentWorld->OnWorldBeginPlay.AddUObject(this, &UParrotAudioSubsystem::WorldBeginPlay);

		// Store the current world settings for pulling relevant audio data
		CurrentWorldSettings = Cast<AParrotWorldSettings>(CurrentWorld->GetWorldSettings());

		if (!CurrentWorldSettings)
		{
			UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::WorldInitialization unable to get parrot world settings. Check world %s"), *CurrentWorld->GetName());
			return; 
		}
	}
}

void UParrotAudioSubsystem::WorldBeginPlay()
{
	// Get the game user settings
	CurrentGameSettings = GetParrotGameSettings();

	// Apply sound settings to world audio device 
	UpdateMixersFromAudioData(); 

	// Optionally bind to parrot game state so the subsystem is informed of state events 
	if ((ParrotGameState = CurrentWorld->GetGameState<AParrotGameState>()))
	{
		ParrotGameState->OnLevelStateChanged.AddDynamic(this, &UParrotAudioSubsystem::GameStateChanged); 
	}

	// In editor, we want to play the music on world begin play
#if WITH_EDITOR
	// If the loading screen is held, the loading screen will notify us via LoadingScreenVisibilityChanged
	// If we are changing levels with no loading screen, just start the music
	if (IsValid(ParrotGameInstance) && !ParrotGameInstance->ShouldHoldLoadingScreen())
	{
		StartDefaultWorldMusic();
	}
#endif
	
}

void UParrotAudioSubsystem::LoadingScreenVisibilityChanged(bool bVisible)
{
	if (!bVisible)
	{
		// Now that the loading screen is no longer visible, play the world music 
		StartDefaultWorldMusic();
	}
	else
	{
		// When a loading screen has popped up, stop any world music 
		if (IsValid(WorldMusicPlayer))
		{
			WorldMusicPlayer->Stop(); 
		}
	}
}

void UParrotAudioSubsystem::UpdateMixersFromAudioData()
{
	if (IsValid(CurrentGameSettings))
	{
		// Apply audio data settings to mixer 
		ApplyVolumeChangeToMix(MainSoundClass, CurrentGameSettings->MainVolume, 0.0f);
		ApplyVolumeChangeToMix(MusicSoundClass, CurrentGameSettings->MusicVolume, 0.0f);
		ApplyVolumeChangeToMix(SFXSoundClass, CurrentGameSettings->SFXVolume, 0.0f);
	}
}

void UParrotAudioSubsystem::GameStateChanged(ELevelState NewLevelState)
{
	if (NewLevelState == ELevelState::Paused || NewLevelState == ELevelState::Running)
	{
		TogglePauseWorldMusic(); 
		return; 
	}

	if (NewLevelState == ELevelState::GameOver)
	{
		StartGameOverMusic();
		return; 
	}

	if (NewLevelState == ELevelState::Victory)
	{
		StartVictoryMusic(); 
		return; 
	}
}

void UParrotAudioSubsystem::StartDefaultWorldMusic()
{
	if (!IsValid(CurrentWorld))
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::StartDefaultWorldMusic CurrentWorld is null"));
		return;
	}

	if (!IsValid(CurrentWorldSettings))
	{
		UE_LOG(LogParrotAudioSubsystem, Error, TEXT("UParrotAudioSubsystem::StartDefaultWorldMusic CurrentWorldSettings is null"));
		return;
	}

	if (!IsValid(CurrentWorldSettings->WorldMusic))
	{
		UE_LOG(LogParrotAudioSubsystem, Warning, TEXT("UParrotAudioSubsystem::StartDefaultWorldMusic WorldMusic is null. Check your world settings setup %s"), *CurrentWorld->GetName());
		return;
	}

	PlayWorldMusic(CurrentWorldSettings->WorldMusic); 
}

void UParrotAudioSubsystem::PlayWorldMusic(USoundBase* Music)
{
	if (!IsValid(Music))
	{
		UE_LOG(LogParrotAudioSubsystem, Warning, TEXT("UParrotAudioSubsystem::PlayWorldMusic, Music parameter is null.")); 
		return; 
	}

	if (!IsValid(WorldMusicPlayer))
	{
		// Spawn our audio component 
		WorldMusicPlayer = UGameplayStatics::CreateSound2D(
			CurrentWorld, /* World Object */
			Music, /* USoundBase* */
			1.0f, /* Volume Multiplier */
			1.0f, /* Pitch Multiplier */
			0.0f, /* Start Time */
			nullptr, /* Concurrency Settings */
			true, /* Persist across level transition */
			false); /* Auto Destroy */

		if (CurrentGameSettings)
		{
			// Note that the reason we don't set the multiplier directly on the audio component is because 
			// the audio data multiplier is applied to the mix the sound is apart of, not the player itself. 
			SetMusicVolume(CurrentGameSettings->MusicVolume);
		}
	}
	
	if (WorldMusicPlayer->bIsPaused)
	{
		WorldMusicPlayer->SetPaused(false);
	}

	if (WorldMusicPlayer->IsPlaying())
	{
		WorldMusicPlayer->Stop();
	}

	// Switch the sound 
	WorldMusicPlayer->Sound = Music;

	// Play the music from the beginning 
	WorldMusicPlayer->Play();
}

void UParrotAudioSubsystem::TogglePauseWorldMusic()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return; 
	}

	// Toggle pause of the music player 
	WorldMusicPlayer->SetPaused(!WorldMusicPlayer->bIsPaused);
}

void UParrotAudioSubsystem::StartVictoryMusic()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return;
	}

	WorldMusicPlayer->Stop();

	if (!IsValid(LevelCompleteMusic))
	{
		// No music found so we'll just stop the music player and return 
		return;
	}

	// Set the new sound and play 
	WorldMusicPlayer->SetSound(LevelCompleteMusic); 
	WorldMusicPlayer->Play(); 
}

void UParrotAudioSubsystem::StartGameOverMusic()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return; 
	}

	WorldMusicPlayer->Stop();

	if (!IsValid(LevelFailMusic))
	{
		// No music found so we'll just stop the music player and return 
		return;
	}

	// Set the new sound and play 
	WorldMusicPlayer->SetSound(LevelFailMusic);
	WorldMusicPlayer->Play();
}

TObjectPtr<UParrotGameUserSettings> UParrotAudioSubsystem::GetParrotGameSettings()
{
	if (GEngine)
	{
		return Cast<UParrotGameUserSettings>(GEngine->GetGameUserSettings()); 
	}

	return nullptr; 
}
