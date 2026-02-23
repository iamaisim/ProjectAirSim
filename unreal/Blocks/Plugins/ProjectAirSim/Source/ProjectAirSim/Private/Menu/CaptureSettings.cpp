#include "Menu/CaptureSettings.h"


void UCaptureSettings::Init(TSharedPtr<FJsonObject> RootIn, USettingsMenu* MenuIn)
{
	this->Root = RootIn;
	this->Menu = MenuIn;
	
	if (!LoadAll()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load all capture setting parameters."));
	}
}

bool UCaptureSettings::LoadAll()
{
	if(!LoadImageType()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load image-type."));
		//return false;
	}
	if(!LoadFOVDegrees()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load fov-degrees."));
		//return false;
	}
	if(!LoadWidth()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load width."));
		//return false;
	}
	if(!LoadHeight()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load height."));
		//return false;
	}
	if(!LoadMotionBlurAmount()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load motion-blur-amount."));
		//return false;
	}
	return true;
}

bool UCaptureSettings::LoadImageType()
{
	double ImageTypeValue;
	if (!UJsonManager::GetNumberField(Root, "image-type", ImageTypeValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'image-type' field."));
		ImageTypeValue = 0;
		return false;
	}

	ImageType = ImageTypeValue;
	return true;
}

bool UCaptureSettings::LoadFOVDegrees()
{
	double FOVDegreesValue;
	if (!UJsonManager::GetNumberField(Root, "fov-degrees", FOVDegreesValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'fov-degrees' field."));
		FOVDegrees = 0.f;
		return false;
	}

	FOVDegrees = FOVDegreesValue;
	return true;
}

bool UCaptureSettings::LoadWidth()
{
	double WidthValue;
	if (!UJsonManager::GetNumberField(Root, "width", WidthValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'width' field."));
		Width = 0;
		return false;
	}

	Width = WidthValue;
	return true;
}

bool UCaptureSettings::LoadHeight()
{
	double HeightValue;
	if (!UJsonManager::GetNumberField(Root, "height", HeightValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'height' field."));
		Height = 0;
		return false;
	}

	Height = HeightValue;
	return true;
}

bool UCaptureSettings::LoadMotionBlurAmount()
{
	double MotionBlurAmountValue;
	if (!UJsonManager::GetNumberField(Root, "motion-blur-amount", MotionBlurAmountValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'motion-blur-amount' field."));
		MotionBlurAmount = -1.f;
		return false;
	}

	MotionBlurAmount = MotionBlurAmountValue;
	return true;
}

void UCaptureSettings::PopulateGUI()
{
	if (Widget->ImageTypeText) Widget->ImageTypeText->SetText(FText::FromString(FString::FromInt(ImageType)));
	if (Widget->FOVDegreesTextBox) Widget->FOVDegreesTextBox->SetText(FText::FromString(FString::SanitizeFloat(FOVDegrees)));
	if (Widget->WidthTextBox) Widget->WidthTextBox->SetText(FText::FromString(FString::FromInt(Width)));
	if (Widget->HeightTextBox) Widget->HeightTextBox->SetText(FText::FromString(FString::FromInt(Height)));
	if (MotionBlurAmount == -1.f) Widget->MotionBlurAmountContainer->SetVisibility(ESlateVisibility::Collapsed);
	else if (Widget->MotionBlurAmountTextBox) Widget->MotionBlurAmountTextBox->SetText(FText::FromString(FString::SanitizeFloat(MotionBlurAmount)));
}

void UCaptureSettings::ApplyChanges()
{
	UJsonManager::SetNumberField(Root, "image-type", ImageType);
	UJsonManager::SetNumberField(Root, "fov-degrees", FOVDegrees);
	UJsonManager::SetNumberField(Root, "width", Width);
	UJsonManager::SetNumberField(Root, "height", Height);
	if (MotionBlurAmount != -1.f) UJsonManager::SetNumberField(Root, "motion-blur-amount", MotionBlurAmount);

}