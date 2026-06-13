// 2024 Secret Dimension, Inc.

#include "Player/ParrotPlayerCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemy/ParrotEnemyCharacterBase.h"
#include "Game/ParrotGameState.h"
#include "Player/ParrotCharacterMovementComponent.h"
#include "TimerManager.h"

void AParrotPlayerCharacter::StopJumping()
{
	// Inform our movement component that the jump input has stopped 
	if (TObjectPtr<UParrotCharacterMovementComponent> ParrotCharacterMovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>())
	{
		ParrotCharacterMovementComponent->StopJumpInput(); 
	}

	// Now we can reset our jump state as normal just like ACharacter::StopJumping() 
	ResetJumpState(); 
}

void AParrotPlayerCharacter::CheckJumpInput(float DeltaTime)
{
	// This function is almost identical to ACharacter::CheckJumpInput.
	// The difference is that we do not increment the jump count an additional time when the player is already falling.
	// This is necessary for recognizing coyote time jumps as the first jump. 

	JumpCurrentCountPreJump = JumpCurrentCount;

	if (TObjectPtr<UParrotCharacterMovementComponent> MovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>())
	{
		if (bPressedJump)
		{
			const bool bDidJump = CanJump() && MovementComponent->DoJump(bClientUpdating, DeltaTime);
			if (bDidJump)
			{
				// Transition from not (actively) jumping to jumping.
				if (!bWasJumping)
				{
					JumpCurrentCount++;
					JumpForceTimeRemaining = GetJumpMaxHoldTime();
					OnJumped();
				}
			}

			bWasJumping = bDidJump;
		}
	}
}

bool AParrotPlayerCharacter::CanJumpInternal_Implementation() const
{
	// Can't jump while dead 
	if (CurrentHitPoints <= 0)
	{
		return false; 
	}

	// The rest of this function is almost identical to ACharacter::CanJumpInternal_Implementation. 
	// The difference is that we do not special case for the first jump when the player is already falling. 
	// This is necessary for recognizing coyote time jumps as the first jump. 

	TObjectPtr<UParrotCharacterMovementComponent> MovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>();

	// Ensure that the CharacterMovement state is valid
	bool bJumpIsAllowed = MovementComponent->CanAttemptJump();

	if (bJumpIsAllowed)
	{
		// Ensure JumpHoldTime and JumpCount are valid.
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			bJumpIsAllowed = JumpCurrentCount < JumpMaxCount;
		}
		else
		{
			// Only consider JumpKeyHoldTime as long as:
			// A) The jump limit hasn't been met OR
			// B) The jump limit has been met AND we were already jumping
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bJumpIsAllowed = bJumpKeyHeld &&
				((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
	}

	return bJumpIsAllowed;
}

bool AParrotPlayerCharacter::IsEnemyJumpValid(UBoxComponent* HurtBox)
{
	// When the player is dead, they can't jump off an enemy
	if (IsDead())
	{
		return false; 
	}

	// If the character isn't falling on Z, we cannot jump off an enemy
	// This prevents cases where a player could intersect the hurt box while ascending 
	if (GetCharacterMovement()->Velocity.Z > 0)
	{
		return false; 
	}

	const FVector& CurrentLocation = GetActorLocation(); 

	const FVector& BoxExtents = HurtBox->GetScaledBoxExtent(); 
	const FVector& BoxCenter = HurtBox->GetComponentLocation(); 

	// We subtract here just to add a buffer around floating point precision 
	const float BoxUpperBoundZ = (BoxCenter.Z + BoxExtents.Z) - UE_KINDA_SMALL_NUMBER; 

	// Character location must be above the upper bound Z for a valid jump 
	if (CurrentLocation.Z < BoxUpperBoundZ)
	{
		return false; 
	}

	return true;
}

void AParrotPlayerCharacter::JumpFromEnemyHurtBox()
{
	if (TObjectPtr<UParrotCharacterMovementComponent> ParrotCharacterMovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>())
	{
		// Perform the jump on the character movement 
		ParrotCharacterMovementComponent->DoEnemyJump();

		// Broadcast to blueprint 
		OnEnemyJump(); 
	}
}

// Called when the game starts or when spawned
void AParrotPlayerCharacter::BeginPlay()
{
	if(TObjectPtr<UParrotCharacterMovementComponent> ParrotCharacterMovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>())
	{
		// Store our starting values as these values can change when powerups are used 
		InitialMaxAcceleration = ParrotCharacterMovementComponent->MaxAcceleration;
		InitialMaxWalkSpeed = ParrotCharacterMovementComponent->MaxWalkSpeed;
	}

	// ECC_GameTraceChannel2 is cannonball.
	// ECC_GameTraceChannel3 is enemy. 
	// Both of these object types can be seen in the DefaultEngine.ini which was set in the project's collision settings. 
	// Custom collision object types can be set in that file or in editor under Edit/Project Settings/Collision
	DefaultCannonballCollisionResponseType = GetCapsuleComponent()->GetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2);
	DefaultEnemyCollisionResponseType = GetCapsuleComponent()->GetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3);

	Super::BeginPlay();
}

void AParrotPlayerCharacter::HitCharacter()
{
	if (bHitInvulnerable)
	{
		return; 
	}

	Super::HitCharacter(); 

	// If the character has died after a hit, we don't want to apply an timers
	if (IsDead())
	{
		return; 
	}

	// Do not allow mid air stuns if the toggle is set to false
	if (!bStunMidAir && GetCharacterMovement()->IsFalling())
	{
		return; 
	}

	// If we have a stun duration specified, start a timer for that 
	if (HitStunDuration > 0.0f)
	{
		// Apply any changes we would like at the beginning of a stun
		// In this case, we're making the player immune to incoming hits and disabling input 
		if (TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->DisableInput(PlayerController); 
		}

		// This function call is a little complicated for beginners but we can break it down. 
		// All Actors in a World have access to the function GetWorldTimeManager. The Timer Manager holds timers for latent behavior.
		// Here we are setting a new timer. TimerHandle_HitStun is a member variable which holds the timer handle of the timer we're creating. 
		// Timer handles are used to edit a timer. An example would be pausing a running timer.
		// Looking at the rest of the function call, we're passing 'this' for the object we're calling the timer on,
		// A reference to the function we want to call when the timer repeats, and the duration which is the 'InRate' parameter. The 'InRate' parameter is the amount of time (in seconds) between timer set and firing. 
		// There are additional parameters for looping or starting on a delay but we are not using them here. 
		// So to summarize - AParrotPlayerCharacter::StopHitStun will be called on 'this' object after the 'Duration'
		GetWorldTimerManager().SetTimer(TimerHandle_HitStun, this, &AParrotPlayerCharacter::StopHitStun, HitStunDuration);
	}

	// If we have a hit invulnerability duration specified, start a timer for that
	if (HitInvulnerabilityDuration > 0.0f)
	{
		bHitInvulnerable = true;
		GetWorldTimerManager().SetTimer(TimerHandle_HitInvulnerability, this, &AParrotPlayerCharacter::StopHitInvulnerability, HitInvulnerabilityDuration);
	}
}

void AParrotPlayerCharacter::HitCharacterWithLaunchForce(const FVector& Force)
{
	if (bHitInvulnerable)
	{
		return; 
	}

	Super::HitCharacterWithLaunchForce(Force); 
}

void AParrotPlayerCharacter::CharacterDeath()
{
	// First disable the player's input on the controller
	if (TObjectPtr<APlayerController> PlayerController = GetController<APlayerController>())
	{
		// The DisableInput function on AActor accepts an optional PlayerController parameter. It will disable
		// input on the given controller, or if none is provided it will disable input on every controller in the world.
		// We only want to disable this pawn's controller input in this case.
		PlayerController->DisableInput(PlayerController);
	}

	// If the character dies mid-air, we should stop any jump behavior 
	if (JumpCurrentCount > 0)
	{
		StopJumping(); 
	}

	// If the character is ascending on Z and then dies, we should clear that velocity here so gravity takes over
	if (TObjectPtr<UParrotCharacterMovementComponent> ParrotCharacterMovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>())
	{
		if (ParrotCharacterMovementComponent->Velocity.Z > 0)
		{
			ParrotCharacterMovementComponent->Velocity.Z = 0; 
		}
	}

	// Inform our game state that this character has died 
	if (TObjectPtr<AParrotGameState> ParrotGameState = GetWorld()->GetGameState<AParrotGameState>())
	{
		ParrotGameState->PlayerDeath(this);
	}

	Super::CharacterDeath();
}

void AParrotPlayerCharacter::AddHitpoints(int32 PointsToAdd)
{
	CurrentHitPoints += PointsToAdd;
	OnHitpointsAdded.Broadcast(); 
}

void AParrotPlayerCharacter::ActivateSpeedPowerup(float Duration, float MaxSpeedMultiplier)
{

	TObjectPtr<UParrotCharacterMovementComponent> MovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>();

	// Update our powerup state bool
	bIsSpeedPowerupActive = true; 

	// Multiply both max acceleration and speed by our powerup multiplier 
	MovementComponent->MaxAcceleration = InitialMaxAcceleration * MaxSpeedMultiplier; 
	MovementComponent->MaxWalkSpeed = InitialMaxWalkSpeed * MaxSpeedMultiplier; 

	// Toggle hit invulnerability for any hit events that could overlap this player
	bHitInvulnerable = true; 

	// Turn collision off on a player capsule for enemy object types 
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECR_Ignore); 

	// Turn collision off on a player capsule for cannonball object types 
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECR_Ignore);

	// Start a timer for the powerup
	GetWorldTimerManager().SetTimer(TimerHandle_SpeedPowerup, this, &AParrotPlayerCharacter::StopSpeedPowerup, Duration); 

	// Note that we did not check bIsSpeedPowerupActive here to see if a timer is already active. This is an important case to cover for sequential pickups.
	// We can avoid this because SetTimer will override whatever is set on TimerHandle_SpeedPowerup. Therefore each call to this function will reset our timer as expected. 
	// For more information on Timers, see the official documentation: https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-timers-in-unreal-engine?application_version=5.3
	
	// Emit to blueprint that our speed powerup has been activated 
	OnSpeedPowerupActivated(Duration); 
}

void AParrotPlayerCharacter::StopSpeedPowerup()
{
	// Restore initial movement values 
	TObjectPtr<UParrotCharacterMovementComponent> MovementComponent = GetCharacterMovement<UParrotCharacterMovementComponent>();
	MovementComponent->MaxAcceleration = InitialMaxAcceleration;
	MovementComponent->MaxWalkSpeed = InitialMaxWalkSpeed;

	// Toggle the powerup state tracking
	bIsSpeedPowerupActive = false;

	// Turn hit invulnerability off for any hit events that could overlap this player
	bHitInvulnerable = false; 

	// Turn collision on for a player capsule for enemy object types 
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, DefaultEnemyCollisionResponseType);

	// Turn collision on for a player capsule for enemy object types 
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, DefaultCannonballCollisionResponseType);
}

void AParrotPlayerCharacter::StopHitStun()
{
	// Note that we don't need to clear the timer here as it is not set to looping. 

	if (TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->EnableInput(PlayerController);
	}
}

void AParrotPlayerCharacter::StopHitInvulnerability()
{
	// Note that we don't need to clear the timer here as it is not set to looping. 

	bHitInvulnerable = false;
}
