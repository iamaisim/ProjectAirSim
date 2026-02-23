#include "Menu/CaptureSettingsWidget.h"
#include "Misc/DefaultValueHelper.h"

void UCaptureSettingsWidget::Init(UCaptureSettings* OwnerIn) 
{
	this->Owner = OwnerIn;

	FOVDegreesTextBox->OnTextCommitted.AddDynamic(this, &UCaptureSettingsWidget::CommitFOVDegrees);
	WidthTextBox->OnTextCommitted.AddDynamic(this, &UCaptureSettingsWidget::CommitWidth);
	HeightTextBox->OnTextCommitted.AddDynamic(this, &UCaptureSettingsWidget::CommitHeight);
	MotionBlurAmountTextBox->OnTextCommitted.AddDynamic(this, &UCaptureSettingsWidget::CommitMotionBlurAmount);
}

void UCaptureSettingsWidget::CommitFOVDegrees(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				if (Value >= 5 && Value <= 170)
				{
					Owner->FOVDegrees = Value;
				}
				
			}
		}
    }
	FOVDegreesTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->FOVDegrees)));
}

void UCaptureSettingsWidget::CommitWidth(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			int32 Value;
			if (FDefaultValueHelper::ParseInt(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				if (Value > 0)
				{
					Owner->Width = Value;
				}
			}
		}
    }
	WidthTextBox->SetText(FText::FromString(FString::FromInt(Owner->Width)));
}

void UCaptureSettingsWidget::CommitHeight(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			int32 Value;
			if (FDefaultValueHelper::ParseInt(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				if (Value > 0)
				{
					Owner->Height = Value;
				}
				
			}
		}
    }
	HeightTextBox->SetText(FText::FromString(FString::FromInt(Owner->Height)));
}

void UCaptureSettingsWidget::CommitMotionBlurAmount(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				if (Value >= 0 && Value <= 1)
				{
					Owner->MotionBlurAmount = Value;
				}
				
			}
		}
    }
	MotionBlurAmountTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->MotionBlurAmount)));
}