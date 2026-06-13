// 2024 Secret Dimension, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ParrotDeveloperSettings.generated.h"

class USoundMix; 

/**
 * An auto discovered settings object specifically for Parrot. We can store any project specific objects and properties here.
 * Those soft object pointers will be written to an .ini file used by the game
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Parrot Project Settings"))
class PARROT_API UParrotDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public: 
	UFUNCTION(BlueprintCallable, Category = "Parrot Developer Settings")
	static const UParrotDeveloperSettings* GetParrotDeveloperSettings(); 

	// Gets the settings container name for the settings, either Project or Editor
	virtual FName GetContainerName() const override;

	// Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc.
	virtual FName GetCategoryName() const override;

	// The unique name for your section of settings, uses the class's FName.
	virtual FName GetSectionName() const override;

#if WITH_EDITOR
	// Gets the section text, uses the classes DisplayName by default.
	virtual FText GetSectionText() const override;

	// Gets the description for the section, uses the classes ToolTip by default.
	virtual FText GetSectionDescription() const;
#endif 

	UFUNCTION(BlueprintCallable, Category = "Parrot Developer Settings")
	bool ShouldSkipLogoTrain();

public:
	// The Default Mix Modifier for Audio 
	UPROPERTY(Config, EditAnywhere, Category = "Parrot Audio Settings|Default Classes")
	TSoftObjectPtr<USoundMix> DefaultSoundMixModifier; 

	// The Main Sound Class for volume changes 
	UPROPERTY(Config, EditAnywhere, Category = "Parrot Audio Settings|Default Classes")
	TSoftObjectPtr<USoundClass> MainSoundClass;

	// The Music Sound Class for volume changes 
	UPROPERTY(Config, EditAnywhere, Category = "Parrot Audio Settings|Default Classes")
	TSoftObjectPtr<USoundClass> MusicSoundClass;

	// The SFX Sound Class for volume changes 
	UPROPERTY(Config, EditAnywhere, Category = "Parrot Audio Settings|Default Classes")
	TSoftObjectPtr<USoundClass> SFXSoundClass;

	// The (optional) victory music for the audio subsystem to play for a world
	UPROPERTY(Config, EditAnywhere, Category = "Parrot Audio Settings|Default Music")
	TSoftObjectPtr<USoundBase> LevelCompleteMusic; 

	// The (optional) failure music for the audio subsystem to play for a world
	UPROPERTY(Config, EditAnywhere, Category="Parrot Audio Settings|Default Music")
	TSoftObjectPtr<USoundBase> LevelFailMusic;

	// An editor time toggle for skipping the logo train when launching from the main menu in editor
	UPROPERTY(Config, EditAnywhere, Category="Parrot Editor Settings")
	bool bSkipLogoTrain; 
};
