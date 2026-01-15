#pragma once

#include "CoreMinimal.h"

#include "Menu/JsonManager.h"
#include "Menu/RobotConfig.h"
#include "Menu/ActorSettingsWidget.h"

#include "ActorSettings.generated.h"

UCLASS()
class PROJECTAIRSIM_API UActorSettings : public UObject {

	GENERATED_BODY()

public:

	void Init(TSharedPtr<FJsonObject> RootIn, USettingsMenu* MenuIn);

	bool LoadAll();
	bool LoadName();
	bool LoadXYZ();
	bool LoadRPYDeg();
	bool LoadRobotConfig();

	void PopulateGUI();

	void ApplyChanges();
	
	UPROPERTY()
	UActorSettingsWidget* Widget;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FVector XYZ;

	UPROPERTY()
	FVector RPYDeg;
	
	UPROPERTY()
	URobotConfig* RobotConfig;

private:

	UPROPERTY()
	USettingsMenu* Menu;

	TSharedPtr<FJsonObject> Root;

	
};