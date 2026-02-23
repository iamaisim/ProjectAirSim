#pragma once

#include "CoreMinimal.h"
#include "Menu/CaptureSettingsWidget.h"
#include "SensorSettingsWidget.generated.h"

class USensorSettings;

UCLASS()
class PROJECTAIRSIM_API USensorSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void Init(USensorSettings* OwnerIn);

	UFUNCTION()
	void CommitType(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitEnabled(bool bIsChecked);
	UFUNCTION()
	void CommitCaptureInterval(const FText& Text, ETextCommit::Type CommitType);
	
	UPROPERTY(meta = (BindWidget))
    UTextBlock* IDText;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* TypeTextBox;

	UPROPERTY(meta = (BindWidget))
    UCheckBox* EnabledCheckBox;
	
	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* CaptureIntervalTextBox;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* CaptureIntervalContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capture Settings")
	TSubclassOf<UCaptureSettingsWidget> CaptureSettingsWidgetClass;

	UPROPERTY(meta = (BindWidget))
    UExpandableArea* EACaptureSettings;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* CaptureSettingsContainer;

private:

	USensorSettings* Owner;
	
};