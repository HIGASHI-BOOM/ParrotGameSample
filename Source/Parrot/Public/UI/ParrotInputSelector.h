// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/Overlay.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "ParrotInputSelector.generated.h"

enum class ECommonInputType : uint8; 
class UCommonInputSubsystem; 


// Log Category for the Parrot Input Selector
DECLARE_LOG_CATEGORY_EXTERN(LogParrotInputSelector, Log, All); 

// Declare delegate types related to input selection 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeyChangeStarted, FText, ActionName, bool, bIsAxis); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKeyChangeCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeyChanged, FKey, NewKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKeySwap);

/**
 * A custom common user widget that can enter a key selection state and then listen for a key press - rebinding it to an Input Action. 
 * Integrates with the CommonUI and EnhancedInput subsystems to work with key profiles and can provide information about mappings. 
 * Examples include the key text or the CommonUI key icon. 
 */
UCLASS()
class PARROT_API UParrotInputSelector : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	// Sets up the input selector with the minimum required input data
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	void InitializeInputData(const UEnhancedPlayerMappableKeyProfile* KeyProfile, const FPlayerKeyMapping& MappingData);

	// Returns the display name text from the specified slot 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	FText GetKeyTextFromSlot(const EPlayerMappableKeySlot InSlot) const;

	// Gets the CommonUI icon given the slot and input type 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	FSlateBrush GetKeyIconFromSlot(const UCommonInputSubsystem* CommonInputSubsystem, const EPlayerMappableKeySlot InSlot, ECommonInputType InputType);

	// Stores the initial key mappings for the input action 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	virtual void StoreInitial();

	// Resets this input selector to the default key mappings for the input action 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	virtual void ResetToDefault(const EPlayerMappableKeySlot InSlot);

	// Resets this input selector to the initial key mappings for the input action
	// Note that this is not the same as the default key mappings. 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	virtual void RestoreToInitial();

	// Remaps the slot with a new key, functionally changing the underlying mapping 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	bool ChangeBinding(int32 InKeyBindSlot, FKey NewKey, bool bUpdateOverlappingKeys = true);

	// Returns true if mapping on this selector has been customized
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	bool IsMappingCustomized() const;

	// Gets the display name of the Input Action 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	FText GetActionMappingDisplayName() const;

	// Gets the display category of the Input Action 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	FText GetActionMappingDisplayCategory() const;

	// Queries the EnhancedInput user settings for a mappable key profile 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	UEnhancedPlayerMappableKeyProfile* FindMappableKeyProfile() const;

	// Gets the Enhanced Input user settings 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	UEnhancedInputUserSettings* GetUserSettings() const;

	// Allows pass through of the parent's mouse button event to this particular input selector 
	UFUNCTION(BlueprintCallable, Category = "Parrot|Input")
	FEventReply OnParentMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent); 

public: 

	// Gets the CommonInput subsystem from the local player 
	TObjectPtr<UCommonInputSubsystem> GetCommonInputSubsystem() const; 

	// Invoked whenever this widget is selected by a mouse button while it has focus
	// Note that this function can also be invoked via "virtual accept" keys on the gamepad. 
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; 

	// Invoked whenever a key is released on this widget while it has focus 
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// Invoked whenever an analog value is changed and this widget has focus 
	virtual FReply NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent) override; 

public:

	// The query options to filter down keys on this selector for
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parrot|Input")
	FPlayerMappableKeyQueryOptions QueryOptions;

	// The keys used to escape the key selection state when it's active
	// Note that these cannot be bound to an action 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parrot|Input")
	TArray<FKey> EscapeKeys; 

	// Text to return when a key selection is in-progress and key text has not been assigned 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parrot|Input")
	FText InProgressSelectionText; 

	// The name of this action's mappings
	UPROPERTY(BlueprintReadOnly, Category = "Parrot|Input")
	FName ActionMappingName;

	// The name to display for this action mapping 
	UPROPERTY(BlueprintReadOnly, Category = "Parrot|Input")
	FText MappingDisplayName; 

	// Whether or not this selector is actively listening for a key press 
	UPROPERTY(BlueprintReadOnly, Category = "Parrot|Input")
	bool bIsSelectingKey;

	// When the key selection has started 
	UPROPERTY(BlueprintAssignable)
	FOnKeyChangeStarted OnKeyChangeStarted; 

	// When the key selection was started but an escape key was hit 
	UPROPERTY(BlueprintAssignable)
	FOnKeyChangeCanceled OnKeyChangeCanceled;

	// When a key selection was started and successfully completed 
	UPROPERTY(BlueprintAssignable)
	FOnKeyChanged OnKeyChanged;

	// When a rebind swaps an already mapped key 
	UPROPERTY(BlueprintAssignable)
	FOnKeySwap OnKeySwapped; 

protected:

	// Returns the mapping row from the player mappable key profile 
	const FKeyMappingRow* FindKeyMappingRow() const;

	// Checks if A and B are the same logical key type 
	bool DoKeyTypesMatch(const FKey& A, const FKey& B);

	// Checks if A and B are the same logical axis type 
	bool DoKeyAxisMatch(const FKey& A, const FKey& B);

	// Checks if the target key is a valid selection 
	bool IsKeySelectable(const FKey& TargetKey);

protected:

	// The profile identifier that this key setting is from 
	FString ProfileIdentifier;

	// Store the initial key mappings that are set on this for each slot 
	TMap<EPlayerMappableKeySlot, FKey> InitialKeyMappings;

	// The current key when an event involving a key is triggered on this widget 
	FKey SelectedKey; 

	// The key used by the query options that determines what type this selector is (gamepad or keyboard/mouse)
	FKey SelectorKeyType; 

	// Whether or not the input action associated with this selector uses a Digital Bool for the input action 
	bool bUsesBooleanValueType; 

	// The threshold at which to check analog inputs are valid for assignment 
	// i.e. Pressing "Up" on the Y-Axis of the left thumbstick needs to surpass this threshold during assignment in order to register. 
	const float AnalogInputThreshold = 0.5f; 
};
