// 2024 Secret Dimension, Inc.
#include "UI/ParrotGameLayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ParrotGameLayout)

void UParrotGameLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		Layers.Add(LayerTag, LayerWidget);
	}
}

TObjectPtr<UCommonActivatableWidgetContainerBase> UParrotGameLayout::GetLayerWidget(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}

void UParrotGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	for (const auto& LayerPair : Layers)
	{
		if (!LayerPair.Value->GetWidgetList().Contains(ActivatableWidget))
		{
			continue;
		}

		LayerPair.Value->RemoveWidget(*ActivatableWidget);
		break;
	}
}
