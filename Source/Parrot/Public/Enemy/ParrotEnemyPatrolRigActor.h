// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParrotEnemyPatrolRigActor.generated.h"

class UParrotEnemyPatrolRigComponent;

/*
* This is an example of an actor class with a default subobject generated at construction time. This is useful
* for guaranteeing there is a required component placed into the hierarchy that cannot be removed.
* 
* This essentially wraps our ParrotEnemyPatrolRigComponent to allow it to be placed anywhere in
* the level. This provides additional flexibility, as we can either place this actor into the level to
* place a patrol at any arbitrary location, or attach the component directly to an object instance
* that already exists in the level.
*/
UCLASS()
class PARROT_API AParrotEnemyPatrolRigActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Default constructor 
	AParrotEnemyPatrolRigActor();

public:	
	// The created patrol rig component that this actor owns 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UParrotEnemyPatrolRigComponent> PatrolRigComponent;
};
