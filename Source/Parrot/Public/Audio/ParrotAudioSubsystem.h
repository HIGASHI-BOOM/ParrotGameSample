// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/WorldInitializationValues.h"
#include "ParrotAudioSubsystem.generated.h"

class AParrotGameState; 
class AParrotWorldSettings;
class UParrotDeveloperSettings;
class UParrotGameInstance; 
class UParrotGameUserSettings; 
enum class ELevelState : uint8; 
struct FWorldInitializationValues;

// Log category for the Parrot Audio Subsystem
DECLARE_LOG_CATEGORY_EXTERN(LogParrotAudioSubsystem, Log, All);

/**
* 
* An audio subsystem that shares the lifetime of the game instance. 
* Can be accessed anywhere that a game instance can be. 
* 
*/
UCLASS()
class PARROT_API UParrotAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	// Initializes the subsystem, binding relevant delegates, and sets up runtime audio data
	void Initialize(FSubsystemCollectionBase& Collection) override; 

	// Updates the save game object with audio system specific data
	UFUNCTION(BlueprintCallable, Category = "Parrot|Audio")
	void SaveAudioSettings();

	// Sets the runtime audio data main volume with mixers 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Audio")
	void SetMainVolume(float NewVolume, float FadeIn = 0.0f); 

	// Sets the runtime audio data main volume with mixers 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Audio")
	void SetMusicVolume(float NewVolume, float FadeIn = 0.0f);

	// Sets the runtime audio data main volume with mixers 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Audio")
	void SetSFXVolume(float NewVolume, float FadeIn = 0.0f);

	// Gets the main volume from game user settings 
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Parrot|Audio")
	float GetMainVolume(); 

	// Gets the music volume from game user settings 
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Parrot|Audio")
	float GetMusicVolume(); 

	// Gets the SFX volume from game user settings 
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Parrot|Audio")
	float GetSFXVolume(); 

	// Returns true when the world music player has been created with a sound 
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Parrot|Audio")
	bool WorldMusicInitialized(); 

	// Manually starts the world music given a sound
	UFUNCTION(BlueprintCallable, Category = "Parrot|Audio")
	void PlaySoundAsWorldMusic(USoundBase* Music); 

protected: 

	// Callback to be notified when the loading screen is shown/hidden
	void LoadingScreenVisibilityChanged(bool bVisible);

	// Callback for world initialization 
	void WorldInitialization(UWorld* World, const FWorldInitializationValues IVS);

	// Callback for when BeginPlay is invoked for the currently loaded world 
	void WorldBeginPlay();

	// Takes the current audio data and applies it across sound mixers 
	void UpdateMixersFromAudioData(); 

	// Changes the volume of a mix given a sound class 
	void ApplyVolumeChangeToMix(USoundClass* TargetSoundClass, float Volume, float FadeIn = 0.0f); 

	// Spawns the WorldMusicPlayer and starts the music defined in world settings 
	void StartDefaultWorldMusic(); 

	void PlayWorldMusic(USoundBase* Music); 

	// Pauses the world music player 
	void TogglePauseWorldMusic();

	// Stops the world music and optionally transitions to victory music 
	void StartVictoryMusic(); 

	// Stops the world music and optionally transitions to victory music 
	void StartGameOverMusic();

	// Gets the parrot game settings from GEngine 
	TObjectPtr<UParrotGameUserSettings> GetParrotGameSettings();

	// Reacts to changes in the game state
	// Must be a UFUNCTION as this is bound to a dynamic mulitcast delegate
	UFUNCTION()
	void GameStateChanged(ELevelState NewLevelState); 

	// The current world where audio is playing 
	TObjectPtr<UWorld> CurrentWorld; 

	// The current world's settings 
	TObjectPtr<AParrotWorldSettings> CurrentWorldSettings; 

	// The audio component that persists between worlds and plays music.  
	// Must be a UPROPERTY so it is not garbage collected. 
	UPROPERTY()
	TObjectPtr<UAudioComponent> WorldMusicPlayer; 

	// The Default Mix Modifier for Audio 
	UPROPERTY()
	TObjectPtr<USoundMix> DefaultSoundMixModifier; 

	// The Main Sound Class for volume changes 
	UPROPERTY()
	TObjectPtr<USoundClass> MainSoundClass; 

	// The Music Sound Class for volume changes 
	UPROPERTY()
	TObjectPtr<USoundClass> MusicSoundClass;

	// The SFX Sound Class for volume changes 
	UPROPERTY()
	TObjectPtr<USoundClass> SFXSoundClass;

	// The music to play when a level is completed
	UPROPERTY()
	TObjectPtr<USoundBase> LevelCompleteMusic;

	// The music to play when a level is failed
	UPROPERTY()
	TObjectPtr<USoundBase> LevelFailMusic; 

	// The current game instance 
	TObjectPtr<UParrotGameInstance> ParrotGameInstance; 

	// The current parrot game state for a world 
	TObjectPtr<AParrotGameState> ParrotGameState; 

	// Delegate handle for world creation 
	FDelegateHandle PostWorldCreationHandle;

	// Delegate handle for world cleanup 
	FDelegateHandle PostWorldCleanupHandle; 

	// Delegate handle for when begin play has been called on a world
	FDelegateHandle OnWorldBeginPlayHandle; 

	// Waits to start the world music until the splash screen is gone 
	FTimerHandle TimerHandle_WorldMusicWait; 

	// The Parrot Game User Settings
	TObjectPtr<UParrotGameUserSettings> CurrentGameSettings; 

	// The Parrot Developer Settings 
	const UParrotDeveloperSettings* CurrentDeveloperSettings; 
};
