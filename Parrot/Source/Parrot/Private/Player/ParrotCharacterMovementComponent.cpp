// 2024 Secret Dimension, Inc.


#include "Player/ParrotCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Player/ParrotPlayerCharacter.h"
#include "Kismet/KismetMathLibrary.h"

UParrotCharacterMovementComponent::UParrotCharacterMovementComponent()
{
	// Cache the default gravity that's set on the movement component 
	DefaultGravityScale = GravityScale; 
}

void UParrotCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	TickMovementTimers(DeltaTime); 
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction); 
}

bool UParrotCharacterMovementComponent::CanAttemptJump() const
{
	if (IsJumpAllowed() && !bWantsToCrouch)
	{
		if (bPerformingEnemyJump)
		{
			return true;
		}

		if (IsFalling() && AirborneTime <= CoyoteTime)
		{
			return true;
		}

		if (IsMovingOnGround())
		{
			return true;
		}
	}

	return false;
}

bool UParrotCharacterMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	if (CharacterOwner && CharacterOwner->CanJump())
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || !FMath::IsNearlyEqual(FMath::Abs(GetGravitySpaceZ(PlaneConstraintNormal)), 1.f))
		{
			// If first frame of DoJump, we want to always inject the initial jump velocity.
			// For subsequent frames, during the time Jump is held, it depends... 
			// bDontFallXXXX == true means we want to ensure the character's Z velocity is never less than JumpZVelocity in this period
			// bDontFallXXXX == false means we just want to leave Z velocity alone and "let the chips fall where they may" (e.g. fall properly in physics)

			// NOTE: 
			// Checking JumpCurrentCountPreJump instead of JumpCurrentCount because Character::CheckJumpInput might have
			// incremented JumpCurrentCount just before entering this function... in order to compensate for the case when
			// on the first frame of the jump, we're already in falling stage. So we want the original value before any 
			// modification here.
			// 
			const bool bFirstJump = (CharacterOwner->JumpCurrentCountPreJump == 0);

			if (bFirstJump)
			{
				// When this is true, we want to continue applying the maximum gravity
				if (!bIgnoreInitialJumpStateReset)
				{
					// Reset our jump state tracking in case we were coming from coyote time 
					AirborneTime = 0.0f;
					FallTime = 0.0f;
					bApplyFallingGravity = false;
				}

				// If we've special cased our jump to perform an enemy specific jump, reset that here
				if (bPerformingEnemyJump)
				{
					bPerformingEnemyJump = false; 
				}

				// The time to reach the apex of our jump is always tied to the length of time the player's allowed to hold jump 
				float TimeToApex = CharacterOwner->JumpMaxHoldTime; 

				// Get the gravity of the movement component, independent of the gravity scale
				// We do this because we're calculating the gravity scale as part of this initial jump velocity 
				TObjectPtr<APhysicsVolume> PhysicsVolume = GetPhysicsVolume();
				float g = PhysicsVolume ? PhysicsVolume->GetGravityZ() : UPhysicsSettings::Get()->DefaultGravityZ;

				// We start with a couple design parameters: 
				// - The apex jump height
				// - The Jump Max Hold Time which is the same as the time it takes to reach the apex
					
				// Using Newton's Second Law of Motion, Force = Mass * Acceleration, 
				// we can calculate the gravity scale and initial force needed to reach a specific height in a fixed amount of time. 
					
				// Acceleration is the rate of change with respect to time. Utilizing acceleration due to gravity with respect to our jump apex time, 
				// we can integrate the equation and then calculate the needed jump force. 
				// 1. Time = Force / Gravity
				// 2. Force = Time * Gravity
				float f = TimeToApex * FMath::Abs(g);

				// Using the newly calculated jump force, we can calculate the height you will reach given gravity = 1
				// We can again integrate for height and use the result to scale our jump force. 
				// 1. Force = Sqrt(2 * Gravity * Height)
				// 2. Height = Force ^ 2 / 2 * Gravity
				float h = FMath::Square(f) / (2.0f * FMath::Abs(g)); 

				// Calculate the scale based on the desired height 
				float JumpHeightGravityScale = ApexJumpHeight / h; 

				// Apply our initial speed to Z
				float TargetJumpSpeed = f * JumpHeightGravityScale;
				// Clamp to ensure we never exceed our max speed 
				Velocity.Z = FMath::Max<FVector::FReal>(Velocity.Z, TargetJumpSpeed);

				// Scale gravity based on our jump 
				GravityScale = JumpHeightGravityScale;

				// Set our state variables 
				// We want to notify when the apex of this jump happens so we set this trigger bool from the base Character Movement component
				bNotifyApex = true; 

				// If we've reached this point, the jump input must be active 
				bJumpInputActive = true; 
			}

			SetMovementMode(MOVE_Falling);
			return true;
		}
	}

	return false;
}

void UParrotCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	// If our movement data is invalid, we can't continue 
	if (!HasValidData())
	{
		return;
	}

	// Apply the base character movement component changes from movement changed 
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode); 

	if (IsFalling())
	{
		AirborneTime = 0.0f;
		bIsAirborne = true;

		// If we walked off the edge of a cliff, have the fall gravity take effect immediately 
		if (!bJumpInputActive)
		{
			bApplyFallingGravity = true;
		}
	
	}
	else if (IsMovingOnGround())
	{
		// Reset our jump state tracking 
		AirborneTime = 0.0f; 
		FallTime = 0.0f; 
		bApplyFallingGravity = false; 
		bIsAirborne = false;
		bJumpInputActive = false; // We should ignore any held jump input if the character has returned to the ground
		bIgnoreInitialJumpStateReset = false; // When we've landed, we no longer need to avoid a reset 
		GravityScale = DefaultGravityScale;
	}
}

void UParrotCharacterMovementComponent::NotifyJumpApex()
{
	// When the character has passed the apex of their jump, we want our fall gravity to start acting on the character
	bApplyFallingGravity = true; 
	Super::NotifyJumpApex(); 
}

void UParrotCharacterMovementComponent::StopJumpInput()
{
	bJumpInputActive = false; 
	
	// If we've released the jump button before reaching the apex, we will apply a gravity multiplier
	// so we attenuate the apex early.
	if (IsFalling() && !bApplyFallingGravity)
	{
		GravityScale *= EarlyReleaseGravityMultiplier;
	}
}

void UParrotCharacterMovementComponent::DoEnemyJump()
{
	// Reset our character's jump state values 
	CharacterOwner->JumpKeyHoldTime = 0.0f;
	CharacterOwner->JumpCurrentCountPreJump = 0;
	CharacterOwner->JumpCurrentCount = 0;
	CharacterOwner->JumpForceTimeRemaining = 0.0f;

	// We want to apply the jump differently depending on whether or not the player is holding down the jump button
	if (const TObjectPtr<AParrotPlayerCharacter> ParrotPlayerCharacter = Cast<AParrotPlayerCharacter>(CharacterOwner))
	{
		if (ParrotPlayerCharacter->IsJumpInputActive() || bJumpInputActive)
		{
			// If the player's holding the jump button, we should reset gravity scale 
			GravityScale = DefaultGravityScale;
		}
		else
		{
			// Gravity should continue being applied at its maximum scale so DoJump should ignore any reset 
			bIgnoreInitialJumpStateReset = true;
		}
	}
	else
	{
		bIgnoreInitialJumpStateReset = true; 
	}
	

	// Allows the 'Can Jump' check to pass done in Do Jump
	bPerformingEnemyJump = true; 

	// Note that parameters are not used in this function but are kept for parent class consistency
	DoJump(false, 0.f); 
}

void UParrotCharacterMovementComponent::TickMovementTimers(float DeltaTime)
{
	if (!bIsAirborne)
	{
		return; 
	}

	// Accumulate airborne time to check against coyote time 
	AirborneTime += DeltaTime; 

	if (bApplyFallingGravity)
	{
		InterpolateFallingGravity(DeltaTime); 
	}
}

void UParrotCharacterMovementComponent::InterpolateFallingGravity(float DeltaTime)
{
	// We have to clamp the alpha otherwise the ease will take us beyond 1.0/target blend value
	float Alpha = FMath::Clamp(FallTime / FallGravityBlendTime, 0.f, 1.f);
	GravityScale = UKismetMathLibrary::Ease(FallBeginGravityScale, FallMaxGravityScale, Alpha, GravityEasingType, FallGravityEaseBlend, FallGravityStep);
	FallTime += DeltaTime;
}
