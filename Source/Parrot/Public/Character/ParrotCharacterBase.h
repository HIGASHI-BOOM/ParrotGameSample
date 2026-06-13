// 2025 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ParrotCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterHitSignature);

/**
 * Characters are Pawns that have a mesh, collision, and built-in movement logic.
 * They are responsible for all physical interaction between the player or AI and the world, and also implement basic networking and input models.
 * They are designed for a vertically-oriented player representation that can walk, jump, fly, and swim through the world using CharacterMovementComponent.
 *
 * Parrot builds on the default Character implementation, overriding where necessary to provide
 * common functionality for all derivatives, such as AParrotPlayerCharacter and AParrotEnemyCharacterBase
 *
 * @see APawn
 * @see https://docs.unrealengine.com/latest/INT/Gameplay/Framework/Pawn/Character/
 */

UCLASS(Abstract, Blueprintable)
class PARROT_API AParrotCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Overridden ACharacter
	// Called when the character has landed on the ground
	virtual void Landed(const FHitResult& Hit) override;

	// Public getter to check the death state
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Parrot|Character|State")
	bool IsDead() { return CurrentHitPoints <= 0; }

	// Applies a hit to this character 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void HitCharacter();

	// Applies a hit with force to the character based on impact 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void HitCharacterWithLaunchForce(const FVector& Force); 

	// Instantly kills this character regardless of current hit point count.
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|State")
	void KillCharacter();

	// Notified by the hit animation that the character's head is fully reeled back
	// so we can play any hit VFX
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Parrot|Character|Combat")
	void OnAnimNotifyHitEffect();

	// Delegate that is fired when the character dies
	UPROPERTY(BlueprintAssignable, Category = "Parrot|Character|Combat")
	FCharacterDeathSignature CharacterDeathDelegate; 

	// Delegate that is fired when the character is hit/attacked
	UPROPERTY(BlueprintAssignable, Category = "Parrot|Character|Combat")
	FCharacterHitSignature CharacterHitDelegate; 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ************
	// Character stats
	// ************
	 
	// This is the authored value of hit points this character will have.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "10", UIMin = "0", UIMax = "10", Category = "Parrot|Character|Stats"))
	int32 HitPoints = 1;

	// This is the working hit point count that is updated when attacked.
	UPROPERTY(BlueprintReadWrite, Category = "Parrot|Character|Stats")
	int32 CurrentHitPoints = 0;

	// C++ logic implementation for when the character dies
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void CharacterDeath();

	// Blueprint event here so our derived blueprint can do anything it needs to
	UFUNCTION(BlueprintImplementableEvent, Category = "Parrot|Character|Combat")
	void OnCharacterDeath();

	// Event to be notified when we are hit during combat. Blueprint Implementable 
	// to keep the implementation easy to tweak/change by design.
	UFUNCTION(BlueprintImplementableEvent, Category = "Parrot|Character|Combat")
	void OnCharacterHit();
};
