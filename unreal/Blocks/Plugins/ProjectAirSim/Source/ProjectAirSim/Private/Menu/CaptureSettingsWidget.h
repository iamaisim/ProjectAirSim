#pragma once

#include "CoreMinimal.h"
#include "CaptureSettingsWidget.generated.h"

class UCaptureSettings;

UCLASS()
class PROJECTAIRSIM_API UCaptureSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void Init(UCaptureSettings* OwnerIn);

	UFUNCTION()
	void CommitFOVDegrees(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitWidth(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitHeight(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitMotionBlurAmount(const FText& Text, ETextCommit::Type CommitType);

	UPROPERTY(meta = (BindWidget))
    UTextBlock* ImageTypeText;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* FOVDegreesTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* WidthTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* HeightTextBox;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* MotionBlurAmountContainer;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* MotionBlurAmountTextBox;

private:

	UCaptureSettings* Owner;
	
};