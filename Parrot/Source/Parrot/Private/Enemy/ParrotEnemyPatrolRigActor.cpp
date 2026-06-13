// 2024 Secret Dimension, Inc.


#include "Enemy/ParrotEnemyPatrolRigActor.h"

#include "Enemy/ParrotEnemyPatrolRigComponent.h"

/*
* All we need to do here is create our default subobject, the ParrotEnemyPatrolRig, and set it as our root component.
* This means that the hierarchy is generated for each instance as we place it into the level.
*/
AParrotEnemyPatrolRigActor::AParrotEnemyPatrolRigActor()
{
	PatrolRigComponent = CreateDefaultSubobject<UParrotEnemyPatrolRigComponent>(TEXT("PatrolRig"));
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	PatrolRigComponent->SetupAttachment(RootComponent);
}
