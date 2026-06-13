// 2024 Secret Dimension, Inc.

#include "ParrotEditorTickActorComponent.h"

UParrotEditorTickActorComponent::UParrotEditorTickActorComponent()
{
	// This code will only run in the editor because this component's module is a part of ParrotEditor.Build.cs 
	// By enabling all of these flags, this component will tick at all times in editor
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bTickInEditor = true;
}

void UParrotEditorTickActorComponent::BeginPlay()
{
	Super::BeginPlay();

	// This enables/disables the tick when PIE is started. 
	// Useful for hiding debug drawing visualizations
	PrimaryComponentTick.SetTickFunctionEnable(ShouldTickDuringPlayInEditor);
}
