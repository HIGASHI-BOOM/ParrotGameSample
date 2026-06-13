// 2024 Secret Dimension, Inc.


#include "UI/ParrotHUD.h"

#include "CommonActivatableWidget.h"
#include "UI/ParrotGameLayout.h"

void AParrotHUD::BeginPlay()
{
	ensureMsgf(IsValid(RootLayoutWidget), TEXT("Invalid root widget in AParrotHUD::BeginPlay - check your class setup in editor"));

	// Assign the widget's owner to the owner of the HUD
	RootLayoutWidget->SetOwningPlayer(GetOwningPlayerController()); 

	// Add the widget to the player's screen as the root widget 
	RootLayoutWidget->AddToPlayerScreen(); 

	// Load and add the first widget on the specified layer
	// Subsequent screens can just be stacked upon the root per layer using PushWidgetToLayerStack
	TSubclassOf<UCommonActivatableWidget> HUDWidgetClass = DefaultWidgetClass.LoadSynchronous();
	DefaultWidget = RootLayoutWidget->PushWidgetToLayerStack(DefaultLayerName, HUDWidgetClass);
}
