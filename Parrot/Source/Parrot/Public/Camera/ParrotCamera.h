// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParrotCamera.generated.h"

class AParrotPlayerCharacter;
class AParrotWorldSettings; 
class UBoxComponent;
class UCameraComponent;
class UParrotCharacterMovementComponent;
class USpringArmComponent;

// This enum tracks which behavior the camera should implement
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	None, 
	Follow, 
	Fixed
};

/*
* 
* This class is used as an abstract base class to create blueprint follow camera actors.
* This allows us to have this logic live here and allow the Camera Subsystem to instantiate
* and set any specific derived camera configuration.
* 
*/
UCLASS(Abstract, Blueprintable)
class PARROT_API AParrotCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AParrotCamera();

	virtual void BeginPlay() override; 

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; 

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Sets the target character that the camera should follow 
	void SetFollowTarget(AParrotPlayerCharacter* Target) { FollowTarget = Target; }

	// Sets the world settings from the world this camera belongs to 
	void SetWorldSettings(AParrotWorldSettings* WorldSettings) { ParrotWorldSettings = WorldSettings; }

	// Sets the camera's movement mode and handles any state changing 
	void SetCameraMode(ECameraMode NewMode); 

	// Sets the camera's fixed point location to interpolate to 
	void SetCameraFixedPointTarget(const FVector& TargetPositon) { FixedPointLocation = TargetPositon; }

	// Gets the camera component's position in world space
	FVector GetCameraComponentWorldPosition();

protected: 
	// Updates the camera behavior when in follow target mode 
	void TickFollowBehavior(float DeltaTime); 

	// Updates the camera behavior when in fixed target mode
	void TickFixedPoint(float DeltaTime); 

	// Calculates the speed along the X-axis that the camera should move 
	void CalculateLateralSpeed();

	// Interpolates to a target position calculating lateral and vertical speeds
	void InterpolateFromFollowTarget(float DeltaTime);

	// Interpolates to the last known follow target location 
	void InterpolateToLastKnownFollowLocation(float DeltaTime); 

	// Handles begin overlaps of the movement trigger box 
	UFUNCTION()
	void MovementTriggerBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Handles exit overlaps of the movement trigger box 
	UFUNCTION() 
	void MovementTriggerBoxOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Handles when the character lands on a surface 
	UFUNCTION()
	void OnCharacterLanded(const FHitResult& Hit); 

protected:
	// A multiplier applied to camera speed used to reach the player's position when intersecting with the movement trigger
	UPROPERTY(EditDefaultsOnly, Category="Parrot|Camera|Follow Settings")
	float LateralFollowSpeedMultiplier = 2.0f;

	// A fixed speed per tick that camera will interpolate on the Z-axis to the follow target's Z
	UPROPERTY(EditDefaultsOnly, Category="Parrot|Camera|Follow Settings")
	float FollowZSpeed = 2.0f; 

	// A fixed speed per tick that camera will interpolate to the fixed point
	UPROPERTY(EditDefaultsOnly, Category="Parrot|Camera|Fixed Point Settings")
	float FixedPointInterpolationSpeed = 2.0f;

	// The default SceneComponent to attach to.
	// Has a transform and supports attachment, but has no rendering or collision capabilities.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	// The spring arm used by the camera. Used to maintain a distance from the parent. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	// The camera component itself which renders a viewpoint 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> CameraComponent;

	// A static mesh which will block the player and prevent them from moving past a certain point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> BlockingMesh;

	// A trigger box that will start interpolating the camera when the player is inside of it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> CameraMovementTriggerBox; 

private: 
	// The current state of the camera 
	ECameraMode CameraMode = ECameraMode::Follow; 

	// Whether or not the player is overlapping the trigger box which moves the camera 
	bool bIsOverlappingMovementBox; 

	// The current speed along the X-Axis the camera is using 
	float CurrentLateralSpeed = 0.0f; 

	// Whether or not the camera has reached the fixed point 
	bool bReachedFixedPoint = false; 

	// The Default Y position of the camera
	// We save this to maintain a consistent camera distance from the player across camera mode changes 
	float CameraDefaultY; 

	// The Z position that the camera should interpolate to when in follow target mode
	float FollowTargetZ;

	// The settings used by the current world
	UPROPERTY(Transient)
	TObjectPtr<AParrotWorldSettings> ParrotWorldSettings;

	// An assigned character to follow
	UPROPERTY(Transient)
	TObjectPtr<AParrotPlayerCharacter> FollowTarget;

	// The follow character's movement component
	UPROPERTY(Transient)
	TObjectPtr<UParrotCharacterMovementComponent> FollowCharacterMovementComponent; 

	// The fixed point location to interpolate to when in fixed point camera mode
	FVector FixedPointLocation; 
};
