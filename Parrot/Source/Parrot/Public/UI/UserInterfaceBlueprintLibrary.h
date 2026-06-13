// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UserInterfaceBlueprintLibrary.generated.h"

class UCommonActivatableWidget;
struct FGameplayTag;

// Log category for the Parrot UI Blueprint Library 
DECLARE_LOG_CATEGORY_EXTERN(LogUserInterfaceBlueprintLibrary, Log, All);

/*
* This is a collection of static functions that provide utility functionality related to UI and are accessible from any Blueprint.
*/
UCLASS()
class PARROT_API UUserInterfaceBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Create a new widget and add to target layer. 
	// This new widget is activated and displayed on player's viewport
	UFUNCTION(BlueprintCallable, Category = "Parrot|UI")
	static UCommonActivatableWidget* PushContentToLayerForPlayer(const APlayerController* PlayerController, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	// Toggles the player's ability to use gamepad navigation in UMG 
	UFUNCTION(BlueprintCallable, Category = "Parrot|UI")
	static void SetNavigationEnabled(bool bIsNavEnabled); 

	// Pulls the project version as an FString from the Project Settings 
	UFUNCTION(BlueprintCallable, Category = "Parrot|UI")
	static FString GetProjectVersion(); 
};
