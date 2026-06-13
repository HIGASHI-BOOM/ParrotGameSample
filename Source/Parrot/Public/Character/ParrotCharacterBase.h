// 2025 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ParrotCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterHitSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterHealthChangedSignature, int32, CurrentHitPoints, int32, MaxHitPoints);

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
	bool IsDead() const { return CurrentHitPoints <= 0; }

	// Returns the character's current health value.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Parrot|Character|Stats")
	int32 GetCurrentHitPoints() const { return CurrentHitPoints; }

	// Returns the character's maximum health value.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Parrot|Character|Stats")
	int32 GetMaxHitPoints() const { return HitPoints; }

	// Returns current health divided by max health for progress bars.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Parrot|Character|Stats")
	float GetHealthPercent() const { return HitPoints > 0 ? static_cast<float>(CurrentHitPoints) / static_cast<float>(HitPoints) : 0.0f; }

	// Applies a default damage hit to this character.
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void HitCharacter();

	// Applies a default damage hit with force to the character based on impact.
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void HitCharacterWithLaunchForce(const FVector& Force); 

	// Applies a specific amount of damage to this character.
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void ApplyDamageToCharacter(UPARAM(meta = (ClampMin = "0")) int32 DamageAmount);

	// Applies a specific amount of damage with force to the character based on impact.
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void ApplyDamageWithLaunchForce(UPARAM(meta = (ClampMin = "0")) int32 DamageAmount, const FVector& Force);

	// Restores a specific amount of health without exceeding max health.
	UFUNCTION(BlueprintCallable, Category = "Parrot|Character|Combat")
	virtual void HealCharacter(UPARAM(meta = (ClampMin = "0")) int32 HealAmount);

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

	// Delegate that is fired whenever current health changes.
	UPROPERTY(BlueprintAssignable, Category = "Parrot|Character|Stats")
	FCharacterHealthChangedSignature CharacterHealthChangedDelegate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ************
	// Character stats
	// ************
	 
	// This is the authored maximum health value this character will have.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Parrot|Character|Stats", meta = (ClampMin = "1", UIMin = "1", UIMax = "1000"))
	int32 HitPoints = 100;

	// This is the default damage amount used by legacy hit calls.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Parrot|Character|Stats", meta = (ClampMin = "0", UIMin = "0", UIMax = "1000"))
	int32 DefaultHitDamage = 10;

	// This is the working health value that is updated when attacked or healed.
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
