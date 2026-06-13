// 2024 Secret Dimension, Inc.


#include "Enemy/ParrotEnemyCharacterBase.h"

#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Enemy/ParrotEnemyAIControllerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/ParrotPlayerCharacter.h"

void AParrotEnemyCharacterBase::BeginPlay()
{
	// Save the original max walk speed and disable movement, we only want to enable it when the player is in the patrol volume.
	if (TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
		InitialMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
	}

	Super::BeginPlay();

	/**
	* As part of BeginPlay here, we have to check if the player is already overlapping our trigger volumes.
	* This is because a "Begin Overlap" event won't be dispatched if the player is spawned inside
	* when the level is started.
	*/
	TArray<AActor*> OverlappingActors;
	
	if (IsValid(AttackTriggerVolume))
	{
		AttackTriggerVolume->GetOverlappingActors(OverlappingActors, AParrotPlayerCharacter::StaticClass());
		if (OverlappingActors.Num() > 0)
		{
			// Set our attack speed multiplier
			SetMovementSpeedMultiplier(AttackSpeedMultiplier);

			// Notify the blueprint of this event
			OnPlayerEnteredAttackTriggerVolume();

			// Pass this event through to the Controller so it can know about it for the behavior tree to use
			if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
			{
				EnemyController->OnPlayerEnteredAttackTriggerVolume();
			}
		}
	}

	if (IsValid(PatrolTriggerVolume))
	{
		PatrolTriggerVolume->GetOverlappingActors(OverlappingActors, AParrotPlayerCharacter::StaticClass());
		if (OverlappingActors.Num() > 0)
		{
			// Enable movement
			if (TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement())
			{
				MovementComponent->SetDefaultMovementMode();
			}

			// Notify the blueprint of this event
			OnPlayerEnteredPatrolTriggerVolume();

			// Pass this event through to the Controller so it can know about it for the behavior tree to use
			if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
			{
				EnemyController->OnPlayerEnteredPatrolTriggerVolume();
			}
		}
	}
}

void AParrotEnemyCharacterBase::BeginDestroy()
{
	// Unbind the trigger volume binds
	if (IsValid(PatrolTriggerVolume))
	{
		PatrolTriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &AParrotEnemyCharacterBase::OnBeginPatrolTriggerOverlap);
		PatrolTriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &AParrotEnemyCharacterBase::OnEndPatrolTriggerOverlap);
	}
	if (IsValid(AttackTriggerVolume))
	{
		AttackTriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &AParrotEnemyCharacterBase::OnBeginAttackTriggerOverlap);
		AttackTriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &AParrotEnemyCharacterBase::OnEndAttackTriggerOverlap);
	}

	Super::BeginDestroy();
}

void AParrotEnemyCharacterBase::HitCharacter()
{
	Super::HitCharacter();

	if (IsDead())
	{
		return;
	}

	// If we have a stun duration specified, start a timer for that 
	if (HitStunDuration > 0.0f)
	{
		// Send the blueprint event
		OnHitStun();

		// Pass this event through to the Controller so it can know about it for the behavior tree to use
		if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
		{
			EnemyController->OnHitStun();
		}

		// Set the timer to notify the character of when the stun timer has elapsed
		GetWorldTimerManager().SetTimer(TimerHandle_HitStun, this, &AParrotEnemyCharacterBase::StopHitStun, HitStunDuration);
	}
}

void AParrotEnemyCharacterBase::HitBeginOverlap(AActor* OverlappedActor, float Force)
{
	// If this enemy is dead, ignore any hurt overlaps
	if (IsDead())
	{
		return;
	}

	// We only care if a player has overlapped here.
	if (TObjectPtr<AParrotPlayerCharacter> Player = Cast<AParrotPlayerCharacter>(OverlappedActor))
	{
		if (Player->IsDead())
		{
			return; 
		}

		if (Force <= 0.0f)
		{
			Player->HitCharacter(); 
		}
		else
		{
			// Calculate a force based on actor positions and the magnitude passed in 
			const FVector HitForce = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal() * Force;
			Player->HitCharacterWithLaunchForce(HitForce);
		} 
	}
}

void AParrotEnemyCharacterBase::HurtBeginOverlap(AActor* OverlappedActor, UBoxComponent* HurtBox, float RecoilForce)
{
	// If this enemy is dead, ignore any hurt overlaps
	if (IsDead())
	{
		return; 
	}

	// We only care if a player has overlapped here.
	if (TObjectPtr<AParrotPlayerCharacter> Player = Cast<AParrotPlayerCharacter>(OverlappedActor))
	{
		// Check if the player can make a valid jump off the enemy based on their state and how they've entered the hurt box. 
		if (!Player->IsEnemyJumpValid(HurtBox))
		{
			return;
		}

		if (RecoilForce > 0.0f)
		{
			// Apply an immediate impulse to the player, opposite of how they hit the enemy 
			FVector HitForce = FVector::ZeroVector;
			HitForce.X = Player->GetVelocity().GetSafeNormal().X * -RecoilForce;
			HitForce.Z = -Player->GetVelocity().Z;

			// We override the XY velocity here to cancel out any velocity the character had 
			Player->LaunchCharacter(HitForce, true, true); // XY Override, Z Override
		}

		// Have the player perform an enemy jump 
		Player->JumpFromEnemyHurtBox();

		// Apply a hit to this enemy  
		HitCharacter(); 
	}
}

FVector AParrotEnemyCharacterBase::GetNextPatrolLocation()
{
	// If we are a non-patrolling enemy, early out
	if (!IsValid(PatrolSpline))
	{
		return FVector::ZeroVector;
	}

	IncrementPatrolPoint();

	// Get the world position of the spline point 
	return PatrolSpline->GetLocationAtSplinePoint(CurrentPatrolPointIndex, ESplineCoordinateSpace::World);
}

void AParrotEnemyCharacterBase::IncrementPatrolPoint()
{
	// If we are a non-patrolling enemy, early out
	if (!IsValid(PatrolSpline))
	{
		return;
	}

	// If we're at the end of the patrol spline, rollover
	if (CurrentPatrolPointIndex + 1 == PatrolSpline->GetNumberOfSplinePoints())
	{
		CurrentPatrolPointIndex = 0;
	}
	else
	{
		CurrentPatrolPointIndex++;
	}
}

void AParrotEnemyCharacterBase::SetMovementSpeedMultiplier(float NewMultiplier)
{
	SpeedMultiplier = NewMultiplier;
	if (TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = InitialMaxWalkSpeed * SpeedMultiplier;
	}
}

void AParrotEnemyCharacterBase::RevertMovementSpeedMultiplier()
{
	SetMovementSpeedMultiplier(1.0f);
}

void AParrotEnemyCharacterBase::Init(FParrotEnemyInitializationArgs InitArgs)
{
	if (auto* PrimitiveComponent = GetComponentByClass<UPrimitiveComponent>())
	{
		PrimitiveComponent->BodyInstance.bLockXTranslation = InitArgs.LockXTransform;
		PrimitiveComponent->BodyInstance.bLockYTranslation = InitArgs.LockYTransform;
		PrimitiveComponent->BodyInstance.bLockZTranslation = InitArgs.LockZTransform;
		PrimitiveComponent->BodyInstance.bLockTranslation = InitArgs.LockXTransform || InitArgs.LockYTransform || InitArgs.LockZTransform;
	}

	// Setup Patrol behavior 
	PatrolSpline = InitArgs.PatrolSpline;
	
	if (IsValid(PatrolSpline))
	{
		// Run any blueprint specific patrol setup behavior 
		OnSetupPatrol();
	}

	// Setup Trigger Volumes to send events 
	PatrolTriggerVolume = InitArgs.PatrolTriggerBox;
	AttackTriggerVolume = InitArgs.AttackTriggerBox;

	if (IsValid(PatrolTriggerVolume))
	{
		PatrolTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AParrotEnemyCharacterBase::OnBeginPatrolTriggerOverlap);
		PatrolTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AParrotEnemyCharacterBase::OnEndPatrolTriggerOverlap);
	}

	if (IsValid(AttackTriggerVolume))
	{
		AttackTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AParrotEnemyCharacterBase::OnBeginAttackTriggerOverlap);
		AttackTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AParrotEnemyCharacterBase::OnEndAttackTriggerOverlap);
	}
}

float AParrotEnemyCharacterBase::GetMovementSpeedMultiplier()
{
	return SpeedMultiplier;
}

void AParrotEnemyCharacterBase::OnBeginPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AParrotPlayerCharacter>(Other))
	{
		// Enable movement
		if (TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement())
		{
			MovementComponent->SetDefaultMovementMode();
		}

		// Notify the blueprint of this event
		OnPlayerEnteredPatrolTriggerVolume();

		// Pass this event through to the Controller so it can know about it for the behavior tree to use
		if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
		{
			EnemyController->OnPlayerEnteredPatrolTriggerVolume();
		}
	}
}

void AParrotEnemyCharacterBase::OnEndPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AParrotPlayerCharacter>(Other))
	{
		// Disable movement
		if (TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement())
		{
			MovementComponent->DisableMovement();
		}

		// Notify the blueprint of this event
		OnPlayerExitedPatrolTriggerVolume();

		// Pass this event through to the Controller so it can know about it for the behavior tree to use
		if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
		{
			EnemyController->OnPlayerExitedPatrolTriggerVolume();
		}
	}
}

void AParrotEnemyCharacterBase::OnBeginAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	if (Cast<AParrotPlayerCharacter>(Other))
	{
		// Set our attack speed multiplier
		SetMovementSpeedMultiplier(AttackSpeedMultiplier);

		// Notify the blueprint of this event
		OnPlayerEnteredAttackTriggerVolume();

		// Pass this event through to the Controller so it can know about it for the behavior tree to use
		if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
		{
			EnemyController->OnPlayerEnteredAttackTriggerVolume();
		}
	}
}

void AParrotEnemyCharacterBase::OnEndAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AParrotPlayerCharacter>(Other))
	{
		// Revert the speed to normal
		RevertMovementSpeedMultiplier();

		// Notify the blueprint of this event
		OnPlayerExitedAttackTriggerVolume();

		// Pass this event through to the Controller so it can know about it for the behavior tree to use
		if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
		{
			EnemyController->OnPlayerExitedAttackTriggerVolume();
		}
	}
}

void AParrotEnemyCharacterBase::StopHitStun()
{
	// Send the blueprint event
	OnStopHitStun();

	// Pass this event through to the Controller so it can know about it for the behavior tree to use
	if (TObjectPtr<AParrotEnemyAIControllerBase> EnemyController = Cast<AParrotEnemyAIControllerBase>(GetController()))
	{
		EnemyController->OnStopHitStun();
	}
}

