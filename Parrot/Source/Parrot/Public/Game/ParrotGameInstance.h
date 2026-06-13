#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LoadingProcessInterface.h"

#include "ParrotGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWindowFocusChanged, bool, bIsFocused);

/**
 * The GameInstance: high-level manager object for an instance of the running game.
 * Spawned at game creation and not destroyed until game instance is shut down.
 * Running as a standalone game, there will be one of these.
 * Running in PIE (play-in-editor) will generate one of these per PIE instance.
 */

/**
* The Loading Process Interface is used for things that might cause loading to happen which requires a loading screen to be displayed.
* In Parrot's case, this is the startup UMG boot splash videos. 
*/

UCLASS(Abstract, Blueprintable)
class UParrotGameInstance : public UGameInstance, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	// Returns true if the boot splash/logo train has been played in this session
	UFUNCTION(BlueprintPure)
	bool HasPlayedBootSplash() const;

	// Records whether or not the boot splash/logo train has been played this session
	UFUNCTION(BlueprintCallable)
	void SetHasPlayedBootSplash(bool bPlayed);

	// Allows the loading screen to be held while something happens (e.g. playing a video)
	// NOTE: be sure to call HoldLoadingScreen(false) when you're done!
	UFUNCTION(BlueprintCallable)
	void HoldLoadingScreen(bool bHold);

	// Returns true if the loading screen has been requested to be held
	UFUNCTION(BlueprintPure)
	bool ShouldHoldLoadingScreen() const;

	//--- GameInstance overrides
	void Init() override;
#if WITH_EDITOR
	FGameInstancePIEResult InitializeForPlayInEditor(int32 PIEInstanceIndex, const FGameInstancePIEParameters& Params) override;
#endif
	//--- End GameInstance

	//--- ILoadingProcessInterface overrides
	bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//--- End ILoadingProcessInterface

public: 
	
	// Blueprint delegate for listening for focus window changes 
	UPROPERTY(BlueprintAssignable)
	FOnWindowFocusChanged OnWindowFocusChanged; 

private: 
	// Binds to slate application and broadcasts an event when the focus has changed 
	void WindowFocusChanged(bool bIsFocused);

private:

	// Whether the game instance has played the boot splash screen yet
	bool bHasPlayedBootSplash = false;

	// Whether or not the loading screen is being held
	bool bHoldLoadingScreen = false;
};
