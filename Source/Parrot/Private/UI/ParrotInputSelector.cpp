// 2024 Secret Dimension, Inc.


#include "UI/ParrotInputSelector.h"
#include "UI/UserInterfaceBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "CommonInputTypeEnum.h"
#include "CommonInputSubsystem.h"

DEFINE_LOG_CATEGORY(LogParrotInputSelector);

namespace Parrot::InputSelector::ErrorMessages
{
	static const FText UnknownMappingName = FText::FromString("Unknown Mapping");
}

void UParrotInputSelector::InitializeInputData(const UEnhancedPlayerMappableKeyProfile* KeyProfile, const FPlayerKeyMapping& MappingData)
{
	check(KeyProfile); 
	ProfileIdentifier = KeyProfile->GetProfileIdString(); 
	
	// In cases where the default key for a mapping is a gamepad axis, we want to change our query on the selector to 
	// only allow axis key inputs. 
	const FKey& DefaultMappingKey = MappingData.GetDefaultKey(); 

	if (DefaultMappingKey.IsValid() && DefaultMappingKey.IsGamepadKey() && DefaultMappingKey.IsAxis1D())
	{
		// Assign the key to match to be an arbitrary 1D gamepad axis key. 
		// Leaving this unchanged will allow any gamepad key to assign here. 
		QueryOptions.KeyToMatch = EKeys::Gamepad_LeftY; 
		QueryOptions.bMatchKeyAxisType = true; 
	}

	SelectorKeyType = QueryOptions.KeyToMatch;

	if (KeyProfile->DoesMappingPassQueryOptions(MappingData, QueryOptions))
	{
		ActionMappingName = MappingData.GetMappingName();
		MappingDisplayName = MappingData.GetDisplayName();
		InitialKeyMappings.Add(MappingData.GetSlot(), MappingData.GetCurrentKey()); 

		// Check the input action for boolean value type. Utilized when processing analog inputs. 
		const UInputAction* Action = MappingData.GetAssociatedInputAction(); 
		if (IsValid(Action))
		{
			bUsesBooleanValueType = (Action->ValueType == EInputActionValueType::Boolean) ? true : false;
		}
	}
}

FText UParrotInputSelector::GetKeyTextFromSlot(const EPlayerMappableKeySlot InSlot) const
{
	// During a selection we do not have the new key name yet 
	if (bIsSelectingKey)
	{
		return InProgressSelectionText; 
	}

	if (const TObjectPtr<UEnhancedPlayerMappableKeyProfile> Profile = FindMappableKeyProfile())
	{
		FPlayerMappableKeyQueryOptions QueryOptionsForSlot = QueryOptions;
		QueryOptionsForSlot.SlotToMatch = InSlot;

		if (const FKeyMappingRow* Row = FindKeyMappingRow())
		{
			for (const FPlayerKeyMapping& Mapping : Row->Mappings)
			{
				if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptionsForSlot))
				{
					// We've passed the query check and GetCurrentKey will always return a valid key 
					// So we can just return the display name of the key here 
					return Mapping.GetCurrentKey().GetDisplayName();
				}
			}
		}
	}

	return EKeys::Invalid.GetDisplayName();
}

FSlateBrush UParrotInputSelector::GetKeyIconFromSlot(const UCommonInputSubsystem* CommonInputSubsystem, const EPlayerMappableKeySlot InSlot, ECommonInputType InputType)
{
	FKey TargetKey = FKey(); 
	if (const TObjectPtr<UEnhancedPlayerMappableKeyProfile> Profile = FindMappableKeyProfile())
	{
		FPlayerMappableKeyQueryOptions QueryOptionsForSlot = QueryOptions;
		QueryOptionsForSlot.SlotToMatch = InSlot;

		if (const FKeyMappingRow* Row = FindKeyMappingRow())
		{
			for (const FPlayerKeyMapping& Mapping : Row->Mappings)
			{
				if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptionsForSlot))
				{
					TargetKey = Mapping.GetCurrentKey(); 
				}
			}
		}
	}

	FSlateBrush SlateBrush;
	if (TargetKey.IsValid() && UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, TargetKey, InputType, CommonInputSubsystem->GetCurrentGamepadName()))
	{
		return SlateBrush;
	}

	return *FStyleDefaults::GetNoBrush();
}

void UParrotInputSelector::StoreInitial()
{
	InitialKeyMappings.Empty(); 
	if (const TObjectPtr<UEnhancedPlayerMappableKeyProfile> Profile = FindMappableKeyProfile())
	{
		if (const FKeyMappingRow* Row = FindKeyMappingRow())
		{
			for (const FPlayerKeyMapping& Mapping : Row->Mappings)
			{
				if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptions))
				{
					ActionMappingName = Mapping.GetMappingName();
					InitialKeyMappings.Add(Mapping.GetSlot(), Mapping.GetCurrentKey());
				}
			}
		}
	}
}

void UParrotInputSelector::ResetToDefault(const EPlayerMappableKeySlot InSlot)
{
	if (TObjectPtr<UEnhancedInputUserSettings> Settings = GetUserSettings())
	{
		bIsSelectingKey = false; 
		FMapPlayerKeyArgs Args = {};
		Args.Slot = InSlot; 
		Args.MappingName = ActionMappingName;

		FGameplayTagContainer FailureReason;
		Settings->ResetAllPlayerKeysInRow(Args, FailureReason);

		if (!FailureReason.IsEmpty())
		{
			UE_LOG(LogParrotInputSelector, Error, TEXT("UParrotInputSelector::ResetToDefault: Failed to reset player keys in row to default."));
		}
	}
}

void UParrotInputSelector::RestoreToInitial()
{
	bIsSelectingKey = false; 
	for (TPair<EPlayerMappableKeySlot, FKey> Pair : InitialKeyMappings)
	{
		ChangeBinding((int32)Pair.Key, Pair.Value, false);
	}
}

bool UParrotInputSelector::ChangeBinding(int32 InKeyBindSlot, FKey NewKey, bool bUpdateOverlappingKeys)
{
	bool bCanAssign = false; 

	const TObjectPtr<UEnhancedPlayerMappableKeyProfile> Profile = FindMappableKeyProfile();

	// Check our query against profile to see if we can assign to this binding 
	if (IsValid(Profile))
	{
		if (const FKeyMappingRow* Row = FindKeyMappingRow())
		{
			for (const FPlayerKeyMapping& Mapping : Row->Mappings)
			{
				if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptions))
				{
					bCanAssign = true; 
					break; 
				}
			}
		}
	}

	bool bUpdatedAlreadyMappedKey = false; 

	// Handle the assignment 
	if (bCanAssign)
	{
		// Setup our mappable key arguments for the action mapping 
		FMapPlayerKeyArgs Args = {};
		Args.MappingName = ActionMappingName;
		Args.Slot = (EPlayerMappableKeySlot)(static_cast<uint8>(InKeyBindSlot));
		
		// If you want to, you can additionally specify this mapping to only be applied to a certain hardware device or key profile
		//Args.ProfileId =
		//Args.HardwareDeviceId =

		if (UEnhancedInputUserSettings* Settings = GetUserSettings())
		{
			if (bUpdateOverlappingKeys)
			{
				// Check if any other mapping is already using the key we're about to assign 
				// Swap it with whatever is already assigned to this action 
				TArray<FName> MatchedMappingNames; 
				int32 MatchCount = Profile->GetMappingNamesForKey(NewKey, MatchedMappingNames); 

				if (MatchCount > 0)
				{
					FName FoundMapping = MatchedMappingNames[0]; 
					
					FMapPlayerKeyArgs BindArgs = {};
					BindArgs.MappingName = FoundMapping;
					BindArgs.Slot = Args.Slot;

					// Unmap the old key 
					FGameplayTagContainer FailureReason;
					Settings->UnMapPlayerKey(BindArgs, FailureReason);

					if (!FailureReason.IsEmpty())
					{
						UE_LOG(LogParrotInputSelector, Error, TEXT("UParrotInputSelector::ChangeBinding: Failed to unmap player key."));
					}

					// Get the current row we're operating on 
					const FKeyMappingRow* MappingRow = FindKeyMappingRow(); 
					FKey ExistingKey; 
					for (const FPlayerKeyMapping& Mapping : MappingRow->Mappings)
					{
						if (Mapping.GetSlot() == Args.Slot)
						{
							ExistingKey = Mapping.GetCurrentKey(); 
							break; 
						}
					}

					// Map the new key 
					BindArgs.NewKey = ExistingKey; 
					FailureReason.Reset(); 
					
					Settings->MapPlayerKey(BindArgs, FailureReason); 

					if (!FailureReason.IsEmpty())
					{
						UE_LOG(LogParrotInputSelector, Error, TEXT("UParrotInputSelector::ChangeBinding: Failed to map player key."));
					}

					bUpdatedAlreadyMappedKey = true; 
				}
			}


			// Unmap the old key 
			FGameplayTagContainer FailureReason;
			Settings->UnMapPlayerKey(Args, FailureReason); 

			if (!FailureReason.IsEmpty())
			{
				UE_LOG(LogParrotInputSelector, Error, TEXT("UParrotInputSelector::ChangeBinding: Failed to unmap player key."));
			}

			FailureReason.Reset(); 

			// Map the new key 
			Args.NewKey = NewKey;
			Settings->MapPlayerKey(Args, FailureReason);

			if (!FailureReason.IsEmpty())
			{
				UE_LOG(LogParrotInputSelector, Error, TEXT("UParrotInputSelector::ChangeBinding: Failed to map player key."));
			}

			if (bUpdatedAlreadyMappedKey)
			{
				// Changing the binding caused a key swap. Broadcast our event to update all impacted selectors. 
				OnKeySwapped.Broadcast(); 
			}

			return true;
		}
	}

	return false;
}

bool UParrotInputSelector::IsMappingCustomized() const
{
	bool bResult = false; 

	if (const TObjectPtr<UEnhancedPlayerMappableKeyProfile> Profile = FindMappableKeyProfile())
	{
		FPlayerMappableKeyQueryOptions QueryOptionsForSlot = QueryOptions;

		if (const FKeyMappingRow* Row = FindKeyMappingRow())
		{
			for (const FPlayerKeyMapping& Mapping : Row->Mappings)
			{
				if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptionsForSlot))
				{
					bResult |= Mapping.IsCustomized();
				}
			}
		}
	}

	return bResult;
}

FText UParrotInputSelector::GetActionMappingDisplayName() const
{
	if (const FKeyMappingRow* Row = FindKeyMappingRow())
	{
		if (Row->HasAnyMappings())
		{
			return Row->Mappings.begin()->GetDisplayName();
		}
	}

	return Parrot::InputSelector::ErrorMessages::UnknownMappingName;
}

FText UParrotInputSelector::GetActionMappingDisplayCategory() const
{
	if (const FKeyMappingRow* Row = FindKeyMappingRow())
	{
		if (Row->HasAnyMappings())
		{
			return Row->Mappings.begin()->GetDisplayCategory();
		}
	}

	return Parrot::InputSelector::ErrorMessages::UnknownMappingName;
}

const FKeyMappingRow* UParrotInputSelector::FindKeyMappingRow() const
{
	if (const TObjectPtr<UEnhancedPlayerMappableKeyProfile> Profile = FindMappableKeyProfile())
	{
		return Profile->FindKeyMappingRow(ActionMappingName);
	}

	ensure(false);
	return nullptr;
}

bool UParrotInputSelector::DoKeyTypesMatch(const FKey& A, const FKey& B)
{
	return A.IsGamepadKey() == B.IsGamepadKey() &&
		A.IsTouch() == B.IsTouch() &&
		A.IsGesture() == B.IsGesture();
}

bool UParrotInputSelector::DoKeyAxisMatch(const FKey& A, const FKey& B)
{
	return A.IsAxis1D() == B.IsAxis1D() &&
		A.IsAxis2D() == B.IsAxis2D() &&
		A.IsAxis3D() == B.IsAxis3D();
}

bool UParrotInputSelector::IsKeySelectable(const FKey& TargetKey)
{
	if (EscapeKeys.Contains(TargetKey) || !TargetKey.IsValid() || !DoKeyTypesMatch(TargetKey, SelectorKeyType) || !DoKeyAxisMatch(TargetKey, SelectorKeyType))
	{
		return false; 
	}

	return true; 
}

FReply UParrotInputSelector::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Gamepad uses an AnalogCursor which can invoke NativeOnMouseButtonUp through virtual accept keys. 
	// To get around this, we need to query the active input method and make sure that only valid input methods can bind to associated input selectors. 
	// i.e. KBM to KBM and Gamepad to Gamepad 

	bool bUsingGamepad = false; 
	if (TObjectPtr<UCommonInputSubsystem> Subsystem = GetCommonInputSubsystem())
	{
		bUsingGamepad = Subsystem->IsInputMethodActive(ECommonInputType::Gamepad); 
		if (SelectorKeyType.IsGamepadKey() && !bUsingGamepad)
		{
			return FReply::Handled(); 
		}

		if (!SelectorKeyType.IsGamepadKey() && bUsingGamepad)
		{
			return FReply::Handled(); 
		}
	}
	else
	{
		UE_LOG(LogParrotInputSelector, Error, TEXT("UParrotInputSelector::NativeOnMouseButtonUp - Unable to get Common Input subsystem from local player.")); 
	}

	if (!bIsSelectingKey)
	{
		bIsSelectingKey = true;
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(false);
		// Pass the current action mapping name and if it's an axis or button so that we can populate the 
		// widget that's awaiting key input 
		OnKeyChangeStarted.Broadcast(FText::FromName(ActionMappingName), SelectorKeyType.IsAxis1D());
		return FReply::Handled();
	}

	// When we're invoking this function through a normal mouse click, we can set the key directly from the mouse event. 
	// However, when we're using a gamepad, we know that a "virtual accept" triggered this which is always bound to gamepad face button bottom. 
	if (!bUsingGamepad)
	{
		SelectedKey = InMouseEvent.GetEffectingButton();
	}
	else
	{
		SelectedKey = EKeys::Gamepad_FaceButton_Bottom; 
	}
	
	if (!IsKeySelectable(SelectedKey))
	{
		bIsSelectingKey = false;
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(true);
		OnKeyChangeCanceled.Broadcast();
		return FReply::Handled();
	}

	bool bHasChanged = false;

	// Stack up the keys to change for a slot
	// The Initial Key Mappings can change as a result of rebinding so we can't just loop them here 
	TArray<int32> Slots; 
	for (TPair<EPlayerMappableKeySlot, FKey> Pair : InitialKeyMappings)
	{
		Slots.Add((int32)Pair.Key);
	}

	while (Slots.Num() > 0)
	{
		bHasChanged |= ChangeBinding(Slots.Pop(), SelectedKey); 
	}

	if (bHasChanged)
	{
		bIsSelectingKey = false;
		OnKeyChanged.Broadcast(SelectedKey);
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(true); 
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UParrotInputSelector::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!bIsSelectingKey)
	{
		return FReply::Handled();
	}
	
	SelectedKey = InKeyEvent.GetKey();

	if (!IsKeySelectable(SelectedKey))
	{
		bIsSelectingKey = false;
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(true);
		OnKeyChangeCanceled.Broadcast();
		return FReply::Handled();
	}

	bool bHasChanged = false; 

	// Stack up the keys to change for a slot
	// The Initial Key Mappings can change as a result of rebinding so we can't just loop them here 
	TArray<int32> Slots;
	for (TPair<EPlayerMappableKeySlot, FKey> Pair : InitialKeyMappings)
	{
		Slots.Add((int32)Pair.Key);
	}

	while (Slots.Num() > 0)
	{
		bHasChanged |= ChangeBinding(Slots.Pop(), SelectedKey);
	}

	if (bHasChanged)
	{
		bIsSelectingKey = false; 
		OnKeyChanged.Broadcast(SelectedKey);
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(true); 
	}

	return Super::NativeOnKeyUp(InGeometry, InKeyEvent); 
}

FReply UParrotInputSelector::NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent)
{
	if (!bIsSelectingKey)
	{
		return Super::NativeOnAnalogValueChanged(InGeometry, InAnalogEvent);
	}

	SelectedKey = InAnalogEvent.GetKey();

	// Check to make sure our selector type matches the incoming key type 
	if (!DoKeyTypesMatch(SelectedKey, SelectorKeyType))
	{
		bIsSelectingKey = false;
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(true);
		OnKeyChangeCanceled.Broadcast();
		return FReply::Handled();
	}

	// NativeOnAnalogValueChanged will be called whenever an input axis value changes.
	// We get both X and Y axes when any thumbstick input is made. 
	float AnalogValue = FMath::Abs(InAnalogEvent.GetAnalogValue()); 

	// Check the value to make sure the player actually wants this axis, otherwise, discard the value and remain in a selecting key state.
	if (AnalogValue < AnalogInputThreshold)
	{
		return FReply::Handled(); 
	}

	AnalogValue = InAnalogEvent.GetAnalogValue(); 

	// Analog stick events and triggers should be processed as virtual press/release key codes for boolean input types 
	if (bUsesBooleanValueType)
	{
		if (SelectedKey == EKeys::Gamepad_LeftX)
		{
			SelectedKey = (AnalogValue < 0) ? EKeys::Gamepad_LeftStick_Left : EKeys::Gamepad_LeftStick_Right;
		}
		else if (SelectedKey == EKeys::Gamepad_LeftY)
		{
			SelectedKey = (AnalogValue > 0) ? EKeys::Gamepad_LeftStick_Up : EKeys::Gamepad_LeftStick_Down; 
		}
		else if (SelectedKey == EKeys::Gamepad_RightX)
		{
			SelectedKey = (AnalogValue < 0) ? EKeys::Gamepad_RightStick_Left : EKeys::Gamepad_RightStick_Right;
		}
		else if (SelectedKey == EKeys::Gamepad_RightY)
		{
			SelectedKey = (AnalogValue > 0) ? EKeys::Gamepad_RightStick_Up : EKeys::Gamepad_RightStick_Down;
		}
		else if (SelectedKey == EKeys::Gamepad_RightTriggerAxis)
		{
			SelectedKey = EKeys::Gamepad_RightTrigger; 
		}
		else if (SelectedKey == EKeys::Gamepad_LeftTriggerAxis)
		{
			SelectedKey = EKeys::Gamepad_LeftTrigger;
		}
	}

	// Check to make sure that the selected key type is still valid
	if (!IsKeySelectable(SelectedKey))
	{
		bIsSelectingKey = false;
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(true);
		OnKeyChangeCanceled.Broadcast();
		return FReply::Handled();
	}

	bool bHasChanged = false;

	// Change the key for the target slot 
	for (TPair<EPlayerMappableKeySlot, FKey> Pair : InitialKeyMappings)
	{
		bHasChanged |= ChangeBinding((int32)Pair.Key, SelectedKey);
	}

	if (bHasChanged)
	{
		bIsSelectingKey = false;
		OnKeyChanged.Broadcast(SelectedKey);
		UUserInterfaceBlueprintLibrary::SetNavigationEnabled(true);
	}

	return FReply::Handled(); 
}

UEnhancedPlayerMappableKeyProfile* UParrotInputSelector::FindMappableKeyProfile() const
{
	if (UEnhancedInputUserSettings* Settings = GetUserSettings())
	{
		return Settings->GetKeyProfileWithId(ProfileIdentifier);
	}

	ensure(false);
	return nullptr;
}

UEnhancedInputUserSettings* UParrotInputSelector::GetUserSettings() const
{
	if (TObjectPtr<ULocalPlayer> LocalPlayer = GetOwningLocalPlayer() )
	{
		// Get the user settings from the subsystem
		if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> System = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			return System->GetUserSettings();
		}
	}

	ensureMsgf(false, TEXT("UParrotInputSelector::GetUserSettings, unable to get user settings from local player enhanced input subsystem"));

	return nullptr;
}

FEventReply UParrotInputSelector::OnParentMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = NativeOnMouseButtonUp(InGeometry, InMouseEvent); 
	return Reply; 
}

TObjectPtr<UCommonInputSubsystem> UParrotInputSelector::GetCommonInputSubsystem() const
{
	if (TObjectPtr<ULocalPlayer> LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<UCommonInputSubsystem>();
	}

	ensureMsgf(false, TEXT("UParrotInputSelector::GetCommonInputSubsystem, unable to get common input local player subsystem"));

	return nullptr; 
}
