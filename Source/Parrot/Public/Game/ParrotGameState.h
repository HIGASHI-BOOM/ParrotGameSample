// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ParrotGameState.generated.h"

/**
* 
* The Game State class is responsible for managing information that is meant to be known to all connected clients and is specific to the game mode. 
* Here you can define state-wide properties such as team score. It is also useful for functions that change the overall state of the game.
* The game state is not specific to any individual player so it is not the best place 
* to store player data. Similar to AParrotGameMode, we're extending AGameStateBase and will create an extended blueprint child class:
* Content/Blueprints/Game/BP_ParrotGameState.uasset
* 
* Official Game State Documentation for reference: https://dev.epicgames.com/documentation/en-us/unreal-engine/game-mode-and-game-state-in-unreal-engine?application_version=5.3
* 
*/


/**
*
* UENUM, UCLASS, GENERATED_BODY, and UFUNCTION are Macros specific to Unreal Engine. These macros provide information to the Unreal Header Tool. 
* UHT allows Unreal to use data at compile time - a good example of this is UFUNCTION(BlueprintCallable) which allows a function to be called in Blueprint. 
* For a list of property specifiers - see the Epic documentation here https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/GameplayArchitecture/Properties/Specifiers/
* For examples of UPROPERTY specifiers, see documentation here https://benui.ca/unreal/uproperty/
*
*/

// This enum tracks the simple state of what is happening in the level.
UENUM(BlueprintType)
enum class ELevelState : uint8
{
	Preload,
	Ready, 
	Running, 
	Paused, 
	GameOver, 
	Victory
};

/** 
* 
* The delegate macros are used to declare a delegate with a number of parameters.Delegates are a useful way to make event driven code.
* Conceptually, they allow functions to subscribe to a "delegate". When that delegate is invoked, so too are the functions.
* Single cast delegates operate as a function pointer and allow one binding. 
* "Multicast" allows multiple functions to be bound to one delegate instance. 
* "Dynamic" allows these delegates to be blueprint assignable as UHT can find them by name. 
* You should consider using non-dynamic multicast delegates outside of this use case. 
* Epic's documentation: https://dev.epicgames.com/documentation/en-us/unreal-engine/dynamic-delegates-in-unreal-engine?application_version=5.4
* BenUI also has some great documentation with examples that is worth checking out: 
* https://benui.ca/unreal/delegates-intro/
* https://benui.ca/unreal/delegates-advanced/
*
*/

// Declare a delegate type with one parameter which will pass the new level state that we just transitioned to. 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLevelStateChanged, ELevelState, NewLevelState);
// Declare a delegate type which will broadcast when the game state is initialized
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameStateInitialized); 


/** 
* 
* Declare our class with the UCLASS Macro
* Here we are specifying that this class should be abstract and never instantiated directly 
* and that it is a type that blueprints can derive from
* 
*/
UCLASS(Abstract, Blueprintable)
class PARROT_API AParrotGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AParrotGameState(); 

	// Updates the game state when a player completes a level
	UFUNCTION(BlueprintCallable)
	void CompleteLevel(AParrotPlayerCharacter* Player);

	// Updates the game state when a player passes out of bounds 
	UFUNCTION(BlueprintCallable)
	void PlayerOutOfBounds(AParrotPlayerCharacter* Player);

	// Updates the game state when a player is killed by enemy combat
	UFUNCTION(BlueprintCallable)
	void PlayerDeath(AParrotPlayerCharacter* Player);

	// Updates the game state when the final boss is killed
	UFUNCTION(BlueprintCallable)
	void BossDefeated(UPARAM(meta=(ClampMin="0"))float Delay);

	// The function we call
	UFUNCTION()
	void BossDefeatedTimerComplete();

	// Handles level state changes and updates the cached level state 
	void SetLevelState(ELevelState NewState);

	// State handling for ready state entry. When the game is initialized but not yet started. 
	void HandleReady(); 

	// Pauses the level state 
	UFUNCTION(BlueprintCallable)
	void PauseGame();

	// Unpauses the level state 
	UFUNCTION(BlueprintCallable)
	void UnPauseGame();

	// Handles logic for when the level is reset. 
	UFUNCTION(BlueprintCallable)
	virtual void ResetCurrentLevel();

	// When the game state has initialized 
	UPROPERTY(BlueprintAssignable)
	FGameStateInitialized OnGameStateInitialized; 

	// Our delegate instance. BlueprintAssignable here exposes this delegate to blueprints 
	// When the level state has changed 
	UPROPERTY(BlueprintAssignable)
	FLevelStateChanged OnLevelStateChanged;
protected:

	// Called by the game mode when play has started 
	virtual void HandleBeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Helper function for checking if the level has ended 
	virtual bool HasMatchEnded() const override; 

protected:

	// The time in seconds that a player is allowed to complete the level before timing out
	// This must be set for the game state to function properly 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(ClampMin = 0.0, UIMin = 0.0, Category="Parrot Game State"))
	float TimePerLevel; 

	// The amount of time left remaining since the player started the level
	UPROPERTY(Transient, BlueprintReadOnly)
	float RemainingTime; 

	// The amount of time accumulated since the player started the level 
	UPROPERTY(Transient, BlueprintReadOnly)
	float AccumulatedTime; 

	// The current simple state of the level 
	UPROPERTY(Transient, BlueprintReadOnly)
	ELevelState LevelState = ELevelState::Preload; 

private:
	// Handle for the timer we set in BossDefeated() 
	FTimerHandle TimerHandle_BossDefeatedDelay;
};
