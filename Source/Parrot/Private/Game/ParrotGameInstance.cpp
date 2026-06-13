
#include "Game/ParrotGameInstance.h"

#include "Framework/Application/SlateApplication.h"
#include "GameMapsSettings.h"
#include "LoadingScreenManager.h"

namespace
{
	static const FName& GetDefaultMapName()
	{
		static FName NAME_DefaultMap = *([]() {
			return FPaths::GetCleanFilename(UGameMapsSettings::GetGameDefaultMap());
		})();
		return NAME_DefaultMap;
	}
}

bool UParrotGameInstance::HasPlayedBootSplash() const
{
	return bHasPlayedBootSplash;
}

void UParrotGameInstance::SetHasPlayedBootSplash(bool bPlayed)
{
	bHasPlayedBootSplash = bPlayed;
}

void UParrotGameInstance::HoldLoadingScreen(bool bHold)
{
	bHoldLoadingScreen = bHold;
}

bool UParrotGameInstance::ShouldHoldLoadingScreen() const
{
	return bHoldLoadingScreen;
}

void UParrotGameInstance::Init()
{
	Super::Init(); // will init all the subsystems as well

	// Bind to slate delegate to be notified of window focus changes 
	// Credit to BenUI https://benui.ca/unreal/window-focus-change/
	FSlateApplication::Get().OnApplicationActivationStateChanged()
		.AddUObject(this, &UParrotGameInstance::WindowFocusChanged);

	if (TObjectPtr<ULoadingScreenManager> LoadingScreenManager = GetSubsystem<ULoadingScreenManager>())
	{
		// Loading processors are stored as weak pointers, so we don't have to worry about cleaning this up
		LoadingScreenManager->RegisterLoadingProcessor(this);
	}
}

#if WITH_EDITOR
FGameInstancePIEResult UParrotGameInstance::InitializeForPlayInEditor(int32 PIEInstanceIndex, const FGameInstancePIEParameters& Params)
{
	FGameInstancePIEResult Result = Super::InitializeForPlayInEditor(PIEInstanceIndex, Params);

	// Don't bother with the boot splash in PIE unless we are intentionally loading into the main menu
	if (GetWorld()->GetName() != GetDefaultMapName())
	{
		bHasPlayedBootSplash = true;
	}

	return Result;
}
#endif

bool UParrotGameInstance::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (ShouldHoldLoadingScreen())
	{
		OutReason = TEXT("ParrotGameInstance is holding the loading screen");
		return true;
	}
	return false;
}

void UParrotGameInstance::WindowFocusChanged(bool bIsFocused)
{
	// Don't pause in the editor, it's annoying
#if !WITH_EDITOR
	if (bIsFocused)
	{
		// Unlimit game FPS
		GEngine->SetMaxFPS(0);
	}
	else
	{
		// Reduce FPS to max 10 while in the background
		GEngine->SetMaxFPS(10.0f);
	}
#endif

	// Broadcast to any listening blueprints 
	OnWindowFocusChanged.Broadcast(bIsFocused); 
}
