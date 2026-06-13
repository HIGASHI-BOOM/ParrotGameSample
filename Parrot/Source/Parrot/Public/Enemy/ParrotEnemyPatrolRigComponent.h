// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "Enemy/ParrotEnemyCharacterBase.h"
#include "ParrotEnemyPatrolRigComponent.generated.h"

class AParrotEnemyCharacterBase;
class UParrotPatrolRigDebugVisualizer;

/* Log Category for the Patrol Rig */
DECLARE_LOG_CATEGORY_EXTERN(LogPatrolRig, Log, All);

/*
* This is an example of a component with default subobjects. It provides a way to have a component that creates and attaches
* required components for its operation into its parent's hierarchy. They cannot be removed unless their parent (this component)
* is removed.
* 
* This is an all-in-one class to allow authoring a patrolling enemy in a single package.
* It can be attached to an actor blueprint to allow placing in the scene, or attached to an existing actor instance
* Since it uses a spline component (which is derived from a scene component), the patrol can be placed anywhere relative
* to the actor parent, and if the actor parent moves, the patrol points will move with it.
* 
*/
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PARROT_API UParrotEnemyPatrolRigComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UParrotEnemyPatrolRigComponent();

	// We have to override these component lifecycle methods so we have the correct
	// timing to manage the default subobjects we create in the constructor

	// Called when the component is created
	virtual void OnComponentCreated() override;

	// Called when the component is destroyed 
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
protected:

	// Called when the component's play begins 
	virtual void BeginPlay() override;

public:	

	// The enemy class to spawn 
	UPROPERTY(EditInstanceOnly)
	TSubclassOf<AParrotEnemyCharacterBase> EnemyToSpawn;
	
	// The enemy initialization arguments 
	// These are used to pass data to the enemy before spawning the actor
	// since the enemy needs to have this information before BeginPlay is invoked
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Category = "Enemy Init"))
	FParrotEnemyInitializationArgs EnemyInitArgs;

protected:

	// The spawned spline that the enemy will patrol 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USplineComponent> PatrolSpline;

	// This is a trigger box that will notify this enemy when the player enters or exits it.
	// Its functionality is intended to toggle the enemy's patrol behavior.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> PatrolTriggerBox;

	// This is a trigger box that will notify this enemy when the player enters or exits it.
	// Its functionality is intended to toggle the enemy's chase or attacking behavior.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> AttackTriggerBox;

	// We are defining this under WITH_EDITORONLY_DATA to make sure that the visualization component is only
	// created in an editor-related context. We don't want or need this to be spawned otherwise.
#if WITH_EDITORONLY_DATA
	// An editor-only component that draws the position and ordering of the waypoints in the PatrolSpline.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UParrotPatrolRigDebugVisualizer> Visualizer;
#endif
};
