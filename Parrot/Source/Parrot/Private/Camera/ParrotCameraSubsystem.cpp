// 2024 Secret Dimension, Inc.


#include "Camera/ParrotCameraSubsystem.h"
#include "Camera/ParrotCamera.h"
#include "Settings/ParrotWorldSettings.h"
#include "Player/ParrotPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogParrotCameraSubsystem)

void UParrotCameraSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	ParrotWorldSettings = Cast<AParrotWorldSettings>(InWorld.GetWorldSettings()); 

	if (!ParrotWorldSettings)
	{
		UE_LOG(LogParrotCameraSubsystem, Error, TEXT("UParrotCameraSubsystem::OnWorldBeginPlay, could not get world settings - check your map settings %s"), *InWorld.GetName()); 
		return; 
	}

	CameraActorClass = ParrotWorldSettings->CameraActorClass; 

	if (!CameraActorClass)
	{
		// This could be intentional and implies that this map should manage its own camera. i.e. The Main Menu map
		UE_LOG(LogParrotCameraSubsystem, Log, TEXT("UParrotCameraSubsystem::OnWorldBeginPlay, no camera class found. Ignoring camera management in %s"), *InWorld.GetName()); 
		return; 
	}

	SetupCamera(); 
}

FVector UParrotCameraSubsystem::GetCameraWorldPosition()
{
	return IsValid(CameraActorInstance) ? CameraActorInstance->GetCameraComponentWorldPosition() : FVector::ZeroVector;
}

void UParrotCameraSubsystem::SetCameraMode(ECameraMode NewMode)
{
	CameraActorInstance->SetCameraMode(NewMode); 
}

void UParrotCameraSubsystem::SetCameraFixedPointTarget(const FVector& TargetPosition)
{
	CameraActorInstance->SetCameraFixedPointTarget(TargetPosition); 
}

bool UParrotCameraSubsystem::IsPlayerCameraOwner(AParrotPlayerCharacter* TargetPlayer)
{
	return TargetPlayer == CameraOwner;
}

void UParrotCameraSubsystem::SetupCamera()
{
	// Get our pawn and pawn location from the local player controller 
	TObjectPtr<UWorld> World = GetWorld();
	TObjectPtr<APlayerController> PlayerController = World->GetFirstLocalPlayerFromController()->GetPlayerController(World);
	TObjectPtr<APawn> Pawn = PlayerController->GetPawn();
	FVector PawnLocation = Pawn->GetActorLocation();

	// Defer spawn our camera, assign the follow target, then finish spawning 
	FTransform SpawnTransform = FTransform(PawnLocation);
	CameraActorInstance = World->SpawnActorDeferred<AParrotCamera>(CameraActorClass.Get(), SpawnTransform);

	CameraActorInstance->SetWorldSettings(ParrotWorldSettings);
	CameraActorInstance->SetCameraMode(ParrotWorldSettings->DefaultCameraMode); 

	// Always set the follow target based on the camera's owner 
	if (TObjectPtr<AParrotPlayerCharacter> TargetPlayer = Cast<AParrotPlayerCharacter>(Pawn))
	{
		CameraOwner = TargetPlayer; 
		CameraActorInstance->SetFollowTarget(TargetPlayer);
	}
	else
	{
		UE_LOG(LogParrotCameraSubsystem, Error, TEXT("UParrotCameraSubsystem::SetupCamera, Could not cast pawn to Parrot Player Character. Check character setup."));
	}
	
	CameraActorInstance->FinishSpawning(SpawnTransform);

	// Blend the player controller's view target with the newly spawned camera 
	PlayerController->SetViewTargetWithBlend(CameraActorInstance);
}
