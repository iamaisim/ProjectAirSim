#include "Menu/ActorSettingsWidget.h"
#include "Misc/DefaultValueHelper.h"

void UActorSettingsWidget::Init(UActorSettings* OwnerIn) 
{
	this->Owner = OwnerIn;
	
	XTextBox->OnTextCommitted.AddDynamic(this, &UActorSettingsWidget::CommitX);
	YTextBox->OnTextCommitted.AddDynamic(this, &UActorSettingsWidget::CommitY);
	ZTextBox->OnTextCommitted.AddDynamic(this, &UActorSettingsWidget::CommitZ);
	
	RollTextBox->OnTextCommitted.AddDynamic(this, &UActorSettingsWidget::CommitRoll);
	PitchTextBox->OnTextCommitted.AddDynamic(this, &UActorSettingsWidget::CommitPitch);
	YawTextBox->OnTextCommitted.AddDynamic(this, &UActorSettingsWidget::CommitYaw);
}

void UActorSettingsWidget::CommitX(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed float: %f"), Value);
				Owner->XYZ.X = Value;
			}
		}
    }
	XTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->XYZ.X)));
}

void UActorSettingsWidget::CommitY(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed float: %f"), Value);
				Owner->XYZ.Y = Value;
			}
		}
    }
	YTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->XYZ.Y)));
}

void UActorSettingsWidget::CommitZ(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed float: %f"), Value);
				Owner->XYZ.Z = Value;
			}
		}
    }
	ZTextBox->SetText(FText::FromString(FString::FromInt(Owner->XYZ.Z)));
}

void UActorSettingsWidget::CommitRoll(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed float: %f"), Value);
				Owner->RPYDeg.X = Value;
			}
		}
    }
	RollTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->RPYDeg.X)));
}

void UActorSettingsWidget::CommitPitch(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed float: %f"), Value);
				Owner->RPYDeg.Y = Value;
			}
		}
    }
	PitchTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->RPYDeg.Y)));
}

void UActorSettingsWidget::CommitYaw(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
    {
        if (!Text.IsEmpty())
		{
			float Value;
			if (FDefaultValueHelper::ParseFloat(Text.ToString(), Value) && Text.ToString().IsNumeric())
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed float: %f"), Value);
				Owner->RPYDeg.Z = Value;
			}
		}
    }
	YawTextBox->SetText(FText::FromString(FString::SanitizeFloat(Owner->RPYDeg.Z)));
}