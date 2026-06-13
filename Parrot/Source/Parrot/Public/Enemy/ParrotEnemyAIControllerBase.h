// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ParrotEnemyAIControllerBase.generated.h"

/**
 * AParrotEnemyAIControllerBase is the base class of controllers for AI-controlled Pawns in Parrot.
 * 
 * Controllers are non-physical actors that can be attached to a pawn to control its actions.
 * AIControllers manage the artificial intelligence for the pawns they control.
 * In networked games, they only exist on the server.
 */
UCLASS(Abstract, Blueprintable)
class PARROT_API AParrotEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()
	
public:
	// Blueprint events so the derived blueprint classes can implement how they want to handle them
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat")
	void OnDeath();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Patrol")
	void OnPlayerEnteredPatrolTriggerVolume();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Patrol")
	void OnPlayerExitedPatrolTriggerVolume();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat")
	void OnPlayerEnteredAttackTriggerVolume();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat")
	void OnPlayerExitedAttackTriggerVolume();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat")
	void OnHitStun();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat")
	void OnStopHitStun();
};
