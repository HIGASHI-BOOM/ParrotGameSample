// 2024 Secret Dimension, Inc.


#include "Enemy/ParrotPatrolRigDebugVisualizer.h"

#include "Components/SplineComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"


FPatrolRigSceneProxy::FPatrolRigSceneProxy(const UPrimitiveComponent* InComponent)
	: FDebugRenderSceneProxy(InComponent)
{
	// Set the view flag name here that we register as a 
	// custom show flag below to tie in visibility of what we draw
	ViewFlagName = TEXT("PatrolRigPoints");
}

void FPatrolRigDrawDelegateHelper::DrawDebugLabels(UCanvas* Canvas, APlayerController* PlayerController)
{
	check(GEngine)

	// You can't paint if you don't have a canvas...
	if (Canvas == nullptr)
	{
		return;
	}

	// Exit if we don't have any spline or spline points to indicate
	if (PatrolSpline == nullptr || PatrolSpline->GetNumberOfSplinePoints() == 0)
	{
		return;
	}

	// Save off the original draw color to revert it later
	const FColor originalDrawColor = Canvas->DrawColor;
	const FSceneView* view = Canvas->SceneView;
	UFont* font = GEngine->GetLargeFont();

	for (int pointIdx = 0; pointIdx < PatrolSpline->GetNumberOfSplinePoints(); ++pointIdx)
	{
		// Set the color based on where we are in the spline
		if (pointIdx == 0)
		{
			// First point is green
			Canvas->SetDrawColor(FColor::Green);
		}
		else if (pointIdx == PatrolSpline->GetNumberOfSplinePoints() - 1)
		{
			// Last point is red
			Canvas->SetDrawColor(FColor::Red);
		}
		else
		{
			// Intermediate points are yellow
			Canvas->SetDrawColor(FColor::Yellow);
		}

		FVector pointPosition = PatrolSpline->GetLocationAtSplinePoint(pointIdx, ESplineCoordinateSpace::World);

		// Raise the position above the point's position for visibility
		pointPosition.Z += 40.f;

		if (view->ViewFrustum.IntersectSphere(pointPosition, 1.0f))
		{
			const FVector screenLocation = Canvas->Project(pointPosition);
			Canvas->DrawText(font, FString::FromInt(pointIdx), screenLocation.X, screenLocation.Y, TextScale, TextScale);
		}
	}

	// Reset the original draw color
	Canvas->SetDrawColor(originalDrawColor);
}

UParrotPatrolRigDebugVisualizer::UParrotPatrolRigDebugVisualizer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Here we register a custom show flag so it appears in the editor viewport's show panel
	// This allows what we draw to be toggled off and on at edit time
	FEngineShowFlags::RegisterCustomShowFlag(TEXT("PatrolRigPoints"), false, SFG_Normal,
		FText::FromString("Patrol Rig Points"));
}

FDebugRenderSceneProxy* UParrotPatrolRigDebugVisualizer::CreateDebugSceneProxy()
{
	// Set the necessary data in our helper so it can be used to generate 
	// the draw information in DrawDebugLabels.
	if (auto* owner = GetOwner())
	{
		if (auto* patrolSpline = owner->GetComponentByClass<USplineComponent>())
		{
			PatrolRigDrawDelegateHelper.PatrolSpline = patrolSpline;
			PatrolRigDrawDelegateHelper.TextScale = TextScale;
		}
	}

	return new FPatrolRigSceneProxy(this);
}
