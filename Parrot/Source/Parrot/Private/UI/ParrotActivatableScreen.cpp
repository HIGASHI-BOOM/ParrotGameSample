// 2024 Secret Dimension, Inc.

#include "UI/ParrotActivatableScreen.h"

#include "EnhancedInputComponent.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Input/UIActionBindingHandle.h"
#include "UI/ParrotGameLayout.h"
#include "UI/ParrotHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ParrotActivatableScreen)

UParrotActivatableScreen::UParrotActivatableScreen()
{
	// Ensure that all activatable screens can handle generic "back" actions by default 
	bIsBackHandler = true;
}

void UParrotActivatableScreen::RefreshUIInputConfig()
{
	if (UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*this))
	{
		ActionRouter->RefreshUIInputConfig();
	}
}

bool UParrotActivatableScreen::NativeOnHandleBackAction()
{
	// Invoke our back event 
	OnGenericBackTriggered(); 

	return true; 
}

void UParrotActivatableScreen::CloseScreen()
{
	// Find the HUD from the owning player controller and remove this widget from its layer
	if (TObjectPtr<APlayerController> PlayerController = GetOwningPlayer())
	{
		if (TObjectPtr<AParrotHUD> ParrotHUD = PlayerController->GetHUD<AParrotHUD>())
		{
			if (TObjectPtr<UParrotGameLayout> Root = ParrotHUD->GetRootLayoutWidget())
			{
				Root->FindAndRemoveWidgetFromLayer(this); 
			}
		}
	}
}
