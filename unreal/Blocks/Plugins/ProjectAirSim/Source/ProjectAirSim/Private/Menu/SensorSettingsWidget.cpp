#include "Menu/SensorSettingsWidget.h"
#include "Misc/DefaultValueHelper.h"

void USensorSettingsWidget::Init(USensorSettings* OwnerIn) 
{
	this->Owner = OwnerIn;

	TypeTextBox->OnTextCommitted.AddDynamic(this, &USensorSettingsWidget::CommitType);
	EnabledCheckBox->OnCheckStateChanged.AddDynamic(this, &USensorSettingsWidget::CommitEnabled);
	CaptureIntervalTextBox->OnTextCommitted.AddDynamic(this, &USensorSettingsWidget::CommitCaptureInterval);
}

void USensorSettingsWidget::CommitType(const FText& Text, ETextCommit::Type ETextCommitType)
{
	if (ETextCommitType == ETextCommit::OnEnter || ETextCommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			FString TextString = Text.ToString().ToLower();
			if (TextString.Equals("airspeed") ||
				TextString.Equals("barometer") ||
				TextString.Equals("camera") ||
				TextString.Equals("gps") ||
				TextString.Equals("imu") ||
				TextString.Equals("lidar") ||
				TextString.Equals("magnetometer") ||
				TextString.Equals("radar")) 
			{
				Owner->Type = TextString;
			}
		}
    }
	TypeTextBox->SetText(FText::FromString(Owner->Type));
}

void USensorSettingsWidget::CommitEnabled(bool bIsChecked) 
{
	Owner->Enabled = bIsChecked;
	EnabledCheckBox->SetIsChecked(bIsChecked);
}

void USensorSettingsWidget::CommitCaptureInterval(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric() && Value > 0)
			{
				Owner->CaptureInterval = Value;
			}
		}
    }
	CaptureIntervalTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->CaptureInterval)));
}