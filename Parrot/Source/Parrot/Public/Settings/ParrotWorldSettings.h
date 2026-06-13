// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "ParrotWorldSettings.generated.h"

enum class ECameraMode : uint8;

/**
 * Actor containing all script accessible world properties. 
 * For Parrot, these are object references that each level will need as we load in.  
 */
UCLASS()
class PARROT_API AParrotWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:

	// The default music for audio subsystem to play for a world once all loading has completed
	// (Can be 'None')
	UPROPERTY(EditDefaultsOnly, Category="Parrot|Audio|Sounds")
	TObjectPtr<USoundBase> WorldMusic; 

	// The default camera actor class that should spawn in the world as part of the camera subsystem 
	// (Can be 'None') 
	UPROPERTY(EditDefaultsOnly, Category="Parrot|Camera")
	TSubclassOf<AActor> CameraActorClass; 

	// The default camera behavior tick mode that the parrot camera should use 
	// (Can be 'None') 
	UPROPERTY(EditDefaultsOnly, Category = "Parrot|Camera")
	ECameraMode DefaultCameraMode;

	// The lower bound at which the camera will stop following player Z 
	UPROPERTY(EditDefaultsOnly, Category="Parrot|Camera")
	float CutoffLowerBoundZ = -400.0f; 

	// The upper bound at which the camera will stop following player Z 
	UPROPERTY(EditDefaultsOnly, Category="Parrot|Camera")
	float CutoffUpperBoundZ = 750.0f; 
};
