// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "ParrotGameUserSettings.generated.h"

/**
 * Stores user settings for a Parrot game client (for example graphics and sound settings), with the ability to save and load to and from a file.
 */
UCLASS(config=GameUserSettings, configdonotcheckdefaults)
class PARROT_API UParrotGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
	
public:

	// The configurable main volume of the game 
	UPROPERTY(Config)
	float MainVolume = 1.0f;

	// The configurable music volume of the game 
	UPROPERTY(Config)
	float MusicVolume = 0.5f; 

	// The configurable SFX volume of the game
	UPROPERTY(Config)
	float SFXVolume = 1.0f; 
};
