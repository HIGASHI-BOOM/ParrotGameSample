// 2024 Secret Dimension, Inc.


#include "Character/ParrotCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


void AParrotCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// If the character is dead and was in the air, we need to stop the character's movement when we land
	// This allows the character's mesh to behave appropriately to land on a platform or the landscape
	// before movement is halted.
	if (IsDead())
	{
		switch (Hit.Component->GetCollisionObjectType())
		{
			// We only care if we have hit world geometry here
		case ECollisionChannel::ECC_WorldStatic:
		case ECollisionChannel::ECC_WorldDynamic:
			if (TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement())
			{
				if (MovementComponent->IsMovingOnGround())
				{
					MovementComponent->DisableMovement();
					GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
			break;
		default:
			// Do nothing
			break;
		}
	}
}

void AParrotCharacterBase::BeginPlay()
{
	// Current hit points should always start at whatever the authored hit points are
	CurrentHitPoints = HitPoints;

	Super::BeginPlay();
}

void AParrotCharacterBase::KillCharacter()
{
	if (!IsDead())
	{
		// Regardless of the current hit points, set them to 0 and then 
		// invoke the death sequence so the expected death events are dispatched
		CurrentHitPoints = 0;
		CharacterDeath();
	}
}

void AParrotCharacterBase::CharacterDeath()
{
	// We want to check if the character is on the ground when death occurs
	// This is so we can disable the movement entirely so the corpse doesn't 
	// move around/fall/etc.
	//
	// If the character is in the air, we don't disable it now. We listen in Landed()
	// so we can disable movement once the character has fallen down.
	if (TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement())
	{
		if (MovementComponent->IsMovingOnGround())
		{
			MovementComponent->DisableMovement();
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// We want to play the hit effect if the character dies. The hit animation
	// won't play to trigger the anim notify, so we call it manually.
	OnAnimNotifyHitEffect();

	// Invoke the BlueprintImplementable Event so the derived blueprint class knows
	OnCharacterDeath();
	// Broadcast the delegate so any subscribed classes or blueprints know
	CharacterDeathDelegate.Broadcast();
}

void AParrotCharacterBase::HitCharacter()
{
	// If a hit event is invoked after the character has died, ignore this call
	if (IsDead())
	{
		return;
	}

	// Subtract from hit points by one but clamp to zero
	CurrentHitPoints = (CurrentHitPoints > 0) ? CurrentHitPoints - 1 : 0; 

	// Invoke the BlueprintImplementable Event so the derived blueprint class knows
	OnCharacterHit();
	// Broadcast the delegate so any subscribed classes or blueprints know
	CharacterHitDelegate.Broadcast();

	// Check if the character has died.
	if (IsDead())
	{
		// Do any relevant death logic 
		CharacterDeath();
	}
}

void AParrotCharacterBase::HitCharacterWithLaunchForce(const FVector& Force)
{
	// If a hit event is invoked after the character has died, ignore this call
	if (IsDead())
	{
		return;
	}

	// Apply an impulse force to the character
	FVector LaunchVelocity = Force;

	// Character should never move along the Y-axis so we zero that out here just in case
	LaunchVelocity.Y = 0.0f; 

	// We override the XY and Z velocity here to cancel out any velocity the character had 
	LaunchCharacter(LaunchVelocity, true, true); // XY Override, Z Override

	// Apply regular hit logic
	HitCharacter(); 
}
