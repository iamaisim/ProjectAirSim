#pragma once
#include "CoreMinimal.h"
#include "Menu/JsonManager.h"
#include "Menu/CaptureSettings.h"
#include "Menu/SensorSettingsWidget.h"

#include "SensorSettings.generated.h"

UCLASS()
class PROJECTAIRSIM_API USensorSettings : public UObject {

	GENERATED_BODY()

public:

	void Init(TSharedPtr<FJsonObject> RootIn, USettingsMenu* MenuIn);

	void PopulateGUI();

	void ApplyChanges();

	bool LoadAll();
	bool LoadID();
    bool LoadType();
    bool LoadEnabled();
	bool LoadCaptureInterval();
    bool LoadCaptureSettings();

	UPROPERTY()
	USensorSettingsWidget* Widget;

	UPROPERTY()
	FString ID;

	UPROPERTY()
	FString Type;
	
	UPROPERTY()
	bool Enabled;

	UPROPERTY()
	float CaptureInterval;

private:
	
	USettingsMenu* Menu;

	TSharedPtr<FJsonObject> Root;

	UPROPERTY()
	TArray<UCaptureSettings*> CaptureSettings;
};