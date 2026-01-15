#pragma once

#include "CoreMinimal.h"
#include "Menu/SensorSettingsWidget.h"
#include "RobotConfigWidget.generated.h"

UCLASS()
class PROJECTAIRSIM_API URobotConfigWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
    UTextBlock* FileNameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensors")
	TSubclassOf<USensorSettingsWidget> SensorSettingsWidgetClass;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* SensorSettingsContainer;

private:

	
	
};