#pragma once

#include "CoreMinimal.h"
#include "Menu/JsonManager.h"
#include "Menu/CaptureSettingsWidget.h"

#include "CaptureSettings.generated.h"

class USettingsMenu;

UCLASS()
class PROJECTAIRSIM_API UCaptureSettings : public UObject {

	GENERATED_BODY()

public:
	
	void Init(TSharedPtr<FJsonObject> RootIn, USettingsMenu* MenuIn);
	
	void PopulateGUI();

	void ApplyChanges();

	bool LoadAll();
	bool LoadImageType();
	bool LoadFOVDegrees();
    bool LoadWidth();
    bool LoadHeight();
    bool LoadMotionBlurAmount();  

	UPROPERTY()
	UCaptureSettingsWidget* Widget;

	UPROPERTY()
	int ImageType;
	
	UPROPERTY()
	float FOVDegrees;
	
	UPROPERTY()
	int32 Width;
	
	UPROPERTY()
	int32 Height;
	
	UPROPERTY()
	float MotionBlurAmount;
	
private:
	
	UPROPERTY()
	USettingsMenu* Menu;

	TSharedPtr<FJsonObject> Root;
	
};
