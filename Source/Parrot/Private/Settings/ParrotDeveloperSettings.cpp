// 2024 Secret Dimension, Inc.


#include "Settings/ParrotDeveloperSettings.h"

const UParrotDeveloperSettings* UParrotDeveloperSettings::GetParrotDeveloperSettings()
{
	return GetDefault<UParrotDeveloperSettings>();
}

FName UParrotDeveloperSettings::GetContainerName() const
{
	static const FName ContainerName("Project");
	return ContainerName;
}

FName UParrotDeveloperSettings::GetCategoryName() const
{
	static const FName EditorCategoryName("Project");
	return EditorCategoryName;
}

FName UParrotDeveloperSettings::GetSectionName() const
{
	static const FName TargetSectionName("Parrot Developer Settings");
	return TargetSectionName;
}

#if WITH_EDITOR
FText UParrotDeveloperSettings::GetSectionText() const
{
	static const FText TargetSectionText = FText::FromString("Parrot Settings"); 
	return TargetSectionText;
}

FText UParrotDeveloperSettings::GetSectionDescription() const
{
	static const FText TargetSectionDescription = FText::FromString("Project settings specific to Parrot");
	return TargetSectionDescription;
}

#endif

bool UParrotDeveloperSettings::ShouldSkipLogoTrain()
{
	bool result = false; 

#if WITH_EDITOR
	result = bSkipLogoTrain;
#endif

	return result;
}