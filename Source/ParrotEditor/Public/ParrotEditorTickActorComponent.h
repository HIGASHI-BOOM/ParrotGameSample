// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActorComponent.h"
#include "ParrotEditorTickActorComponent.generated.h"

/**
 * This is a base class intended to use to derive blueprints for editor use that require edit-time ticking.
 * An example being using debug drawing calls to visualize actors at edit time.
 * Blueprintable here exposes the component as a BP base class. 
 * BlueprintType allows it to be stored in BP variables .
 * BlueprintSpawnableComponent allows it to be spawned by blueprint actors.
 */
UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UParrotEditorTickActorComponent : public UEditorUtilityActorComponent
{
	GENERATED_BODY()

public:
	// This will enable our editor tick functionality
	UParrotEditorTickActorComponent();

protected:
	// This will disable the tick so during play-in-editor, visualizations are not shown
	virtual void BeginPlay() override;

	// This flag will allow us to configure whether our blueprints continue ticking in PIE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Category = "EditorTick"))
	bool ShouldTickDuringPlayInEditor;
};
