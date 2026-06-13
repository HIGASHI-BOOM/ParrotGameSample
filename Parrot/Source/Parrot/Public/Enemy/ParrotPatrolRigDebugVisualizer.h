// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawComponent.h"
#include "ParrotPatrolRigDebugVisualizer.generated.h"

class USplineComponent;

// This derivative of the FDebugRenderSceneProxy allows us to register 
// with the show flag to draw our text at edit time
class FPatrolRigSceneProxy : public FDebugRenderSceneProxy
{
public:
	FPatrolRigSceneProxy(const UPrimitiveComponent* InComponent);
};

// This derivative of the DebugDrawDelegateHelper is where we get the delegate invoked
// to give us the opportunity to draw whatever debug stuff we want (in DrawDebugLabels)
class FPatrolRigDrawDelegateHelper : public FDebugDrawDelegateHelper
{
public:
	virtual void DrawDebugLabels(UCanvas* Canvas, APlayerController* PlayerController) override;

	float TextScale;
	USplineComponent* PatrolSpline;
};

/**
 * This derivative of the UDebugDrawComponent gives us a component to attach to any actor we want
 * and provides the necessary methods to tie together the ability to draw debug text in the editor viewport
 * at edit time. This can also be expanded to draw shapes such as lines, spheres, cones, etc.
 * to visualize whatever you need to at edit time to represent your actor.
 */
UCLASS(ClassGroup = Editor, meta = (BlueprintSpawnableComponent))
class PARROT_API UParrotPatrolRigDebugVisualizer : public UDebugDrawComponent
{
	GENERATED_BODY()

public:

	// The scale of text to display 
	UPROPERTY(EditDefaultsOnly)
	float TextScale = 5.f;

public:
	// Default constructor 
	UParrotPatrolRigDebugVisualizer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	virtual FDebugDrawDelegateHelper& GetDebugDrawDelegateHelper() override { return PatrolRigDrawDelegateHelper; }

	FPatrolRigDrawDelegateHelper PatrolRigDrawDelegateHelper;
};
