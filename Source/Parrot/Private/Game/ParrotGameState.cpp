// 2024 Secret Dimension, Inc.

#include "Game/ParrotGameState.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

AParrotGameState::AParrotGameState()
{
	// Game state does not tick by default but for our game we would like it to. 
	PrimaryActorTick.bCanEverTick = true;
}

void AParrotGameState::CompleteLevel(AParrotPlayerCharacter* Player)
{
	SetLevelState(ELevelState::Victory); 
}

void AParrotGameState::PlayerOutOfBounds(AParrotPlayerCharacter* Player)
{
	SetLevelState(ELevelState::GameOver); 
}

void AParrotGameState::PlayerDeath(AParrotPlayerCharacter* Player)
{
	SetLevelState(ELevelState::GameOver);
}

void AParrotGameState::BossDefeated(float Delay)
{
	// If there's no delay, invoke the function immediately
	if (Delay <= 0.f)
	{
		BossDefeatedTimerComplete();
		return;
	}

	// Set the timer to delay entering the victory state
	GetWorldTimerManager().SetTimer(TimerHandle_BossDefeatedDelay, this, &AParrotGameState::BossDefeatedTimerComplete, Delay);
}

void AParrotGameState::BossDefeatedTimerComplete()
{
	SetLevelState(ELevelState::Victory);
}

void AParrotGameState::SetLevelState(ELevelState NewState)
{
	// Don't continue if we haven't actually changed state 
	if (LevelState == NewState)
	{
		return;
	}

	// Update our level state 
	LevelState = NewState;

	// Run any C++ specific state handling logic 
	switch (LevelState)
	{
	case ELevelState::GameOver:
	case ELevelState::Victory:
	case ELevelState::Paused: 
	case ELevelState::Running:
		// No native implementation needed here but could be added as needed
		break;
	case ELevelState::Ready:
		HandleReady(); 
		break;
	default:
		break;
	}

	// Broadcast that the level state has changed to our Blueprint delegate 
	OnLevelStateChanged.Broadcast(LevelState); 
}

void AParrotGameState::HandleReady()
{
	// Initialize our transient properties when a level is loaded but before it starts. 
	RemainingTime = TimePerLevel;
	AccumulatedTime = 0; 

	SetLevelState(ELevelState::Running);
}

void AParrotGameState::PauseGame()
{
	if (LevelState == ELevelState::Running)
	{
		SetLevelState(ELevelState::Paused); 
	}
}

void AParrotGameState::UnPauseGame()
{
	if (LevelState == ELevelState::Paused)
	{
		SetLevelState(ELevelState::Running); 
	}
}

void AParrotGameState::ResetCurrentLevel()
{
	UWorld* World = GetWorld();

	// Get the map name and remove the level streaming prefix. 
	// This is functionally equivalent to UGameplayStatics::GetCurrentLevelName available in Blueprint.
	FString LevelName = World->GetMapName();
	LevelName.RemoveFromStart(World->StreamingLevelsPrefix);

	// Functionally equivalent to UGameplayStatics::OpenLevel
	const ETravelType TravelType = TRAVEL_Absolute; 
	FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);

	// Unreal uses URLs to lookup resources. In this case, we're getting a map file through the level name itself. 
	FURL TestURL(&WorldContext.LastURL, *LevelName, TravelType);
	if (TestURL.IsLocalInternal())
	{
		// Make sure the file exists if we are opening a local file
		if (!GEngine->MakeSureMapNameIsValid(TestURL.Map))
		{
			UE_LOG(LogLevel, Warning, TEXT("WARNING: The map '%s' does not exist."), *TestURL.Map);
		}
	}

	GEngine->SetClientTravel(World, *LevelName, TravelType);
}

void AParrotGameState::HandleBeginPlay()
{
	// When we've finished constructing all our objects and the world is initialized, we can move to our ready state. 
	SetLevelState(ELevelState::Ready);

	// Broadcast that the game state's begin play has been called 
	OnGameStateInitialized.Broadcast(); 

	Super::HandleBeginPlay(); 
}

void AParrotGameState::Tick(float DeltaTime)
{
	// Tick down our time only if the game is in a running state. 
	if (LevelState == ELevelState::Running)
	{
		RemainingTime -= DeltaTime; 
		AccumulatedTime += DeltaTime; 

		if (RemainingTime <= 0)
		{
			// When time runs out, the game is over
			RemainingTime = 0; 
			SetLevelState(ELevelState::GameOver);
		}
	}

	Super::Tick(DeltaTime); 
}

bool AParrotGameState::HasMatchEnded() const
{
	return (LevelState == ELevelState::GameOver) || (LevelState == ELevelState::Victory);
}
