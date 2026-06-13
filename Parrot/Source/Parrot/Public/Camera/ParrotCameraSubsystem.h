// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ParrotCameraSubsystem.generated.h"

class AParrotWorldSettings; 
class AParrotCamera; 
class AParrotPlayerCharacter; 
enum class ECameraMode : uint8; 

// Log category for the Parrot Camera Subsystem 
DECLARE_LOG_CATEGORY_EXTERN(LogParrotCameraSubsystem, Log, All); 

/**
* 
* A subsystem that interacts with the game camera.
* Shares the lifetime of the current world. 
* 
*/
UCLASS()
class PARROT_API UParrotCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public: 

	// Invoked when Begin Play is called on the world
	void OnWorldBeginPlay(UWorld& InWorld) override;

	// Gets the world position of the camera component 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Camera")
	FVector GetCameraWorldPosition();

	// Sets the camera's movement mode 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Camera")
	void SetCameraMode(ECameraMode NewMode); 

	// Sets the fixed location that the camera should interpolate to 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Camera")
	void SetCameraFixedPointTarget(const FVector& TargetPosition);

	// Returns true when the player character is the owning player of this camera system 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Camera")
	bool IsPlayerCameraOwner(AParrotPlayerCharacter* TargetPlayer); 

protected:

	// Spawns the camera from the camera class and initializes blend with local player
	void SetupCamera(); 

	// The settings used by the current world
	TObjectPtr<AParrotWorldSettings> ParrotWorldSettings; 

	// The camera actor class to spawn based on the world settings 
	TSubclassOf<AParrotCamera> CameraActorClass;

	// The instance of the camera actor 
	TObjectPtr<AParrotCamera> CameraActorInstance;

	// The pawn that is the focus of this camera 
	TObjectPtr<AParrotPlayerCharacter> CameraOwner; 
};
