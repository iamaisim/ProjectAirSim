#pragma once

#include "CoreMinimal.h"
#include "Menu/JsonManager.h"
#include "Menu/ActorSettings.h"

#include "SceneConfig.generated.h"

UCLASS()
class PROJECTAIRSIM_API USceneConfig : public UObject
{

	GENERATED_BODY()

public:

	void Init(TSharedPtr<FJsonObject> RootIn, const FString& FileNameIn, USettingsMenu* MenuIn);

	bool LoadActors();

	void PopulateGUI();

	void ApplyChanges();
	
	UPROPERTY()
	FString FileName;

	UPROPERTY()
	TArray<UActorSettings*> Actors;

private:

	TSharedPtr<FJsonObject> Root;

	UPROPERTY()
	USettingsMenu* Menu;

};