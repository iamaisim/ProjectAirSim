#include "Menu/SensorSettings.h"

void USensorSettings::Init(TSharedPtr<FJsonObject> RootIn, USettingsMenu* MenuIn)
{
	this->Root = RootIn;
	this->Menu = MenuIn;

	if (!LoadAll()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load all sensor settings."));
	}
}

bool USensorSettings::LoadAll()
{
	if(!LoadID()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load id."));
		//return false;
	}
	if(!LoadType()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load type."));
		//return false;
	}
	if(!LoadEnabled()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load enabled."));
		//return false;
	}
	if(!LoadCaptureInterval()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load capture-interval."));
		//return false;
	}
	if(!LoadCaptureSettings()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load capture-settings."));
		//return false;
	}
	return true;
}

bool USensorSettings::LoadID()
{
	FString IDValue;
	if (!UJsonManager::GetStringField(Root, "id", IDValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'id' field."));
		ID = "";
		return false;
	}

	ID = IDValue;
	return true;
}

bool USensorSettings::LoadType()
{
	FString TypeValue;
	if (!UJsonManager::GetStringField(Root, "type", TypeValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'type' field."));
		Type = "";
		return false;
	}
	if (!TypeValue.Equals("airspeed", ESearchCase::CaseSensitive) &&
		!TypeValue.Equals("barometer", ESearchCase::CaseSensitive) &&
		!TypeValue.Equals("camera", ESearchCase::CaseSensitive) &&
		!TypeValue.Equals("gps", ESearchCase::CaseSensitive) &&
		!TypeValue.Equals("imu", ESearchCase::CaseSensitive) &&
		!TypeValue.Equals("lidar", ESearchCase::CaseSensitive) &&
		!TypeValue.Equals("magnetometer", ESearchCase::CaseSensitive) &&
		!TypeValue.Equals("radar", ESearchCase::CaseSensitive))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid type: %s"), *(TypeValue));
		Type = "";
		return false;
	}

	Type = TypeValue;
	return true;
}

bool USensorSettings::LoadEnabled()
{
	bool EnabledValue;
	if (!UJsonManager::GetBoolField(Root, "enabled", EnabledValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'enabled' field."));
		Enabled = false;
		return false;
	}

	Enabled = EnabledValue;
	return true;
}

bool USensorSettings::LoadCaptureInterval()
{
	double CaptureIntervalValue;
	if (!UJsonManager::GetNumberField(Root, "capture-interval", CaptureIntervalValue)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'capture-interval' field."));
		CaptureInterval = -1.f;
		return false;
	}

	CaptureInterval = CaptureIntervalValue;
	return true;
}

bool USensorSettings::LoadCaptureSettings() 
{
	TArray<TSharedPtr<FJsonValue>> CaptureSettingsArray;
    if (!UJsonManager::GetObjectArray(Root, TEXT("capture-settings"), CaptureSettingsArray)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Sensor does not contain capture-settings."));
		return false;
	}

	for (const auto& CaptureSettingsObject : CaptureSettingsArray) 
	{
        TSharedPtr<FJsonObject> CaptureSettingsRoot = CaptureSettingsObject->AsObject();
		if (!CaptureSettingsRoot.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Capture Settings."));
            continue;
        }
		UCaptureSettings* CaptureSetting = NewObject<UCaptureSettings>(this);
		CaptureSetting->Init(CaptureSettingsRoot, Menu);
		CaptureSettings.Add(CaptureSetting);
	}
	
	return true;
}

void USensorSettings::PopulateGUI()
{
	if (Widget->IDText) Widget->IDText->SetText(FText::FromString(ID));
	if (Widget->TypeTextBox) Widget->TypeTextBox->SetText(FText::FromString(Type));
	if (Widget->EnabledCheckBox) Widget->EnabledCheckBox->SetIsChecked(Enabled);
	if (CaptureInterval == -1.f) Widget->CaptureIntervalContainer->SetVisibility(ESlateVisibility::Collapsed);
	else if (Widget->CaptureIntervalTextBox) Widget->CaptureIntervalTextBox->SetText(FText::FromString(FString::SanitizeFloat(CaptureInterval)));

	Widget->CaptureSettingsContainer->ClearChildren();

	for (auto CaptureSetting : CaptureSettings) 
	{
		if (Widget->EACaptureSettings) Widget->EACaptureSettings->SetVisibility(ESlateVisibility::Visible);
		CaptureSetting->Widget = CreateWidget<UCaptureSettingsWidget>(Widget, Widget->CaptureSettingsWidgetClass);
		CaptureSetting->Widget->Init(CaptureSetting);
        Widget->CaptureSettingsContainer->AddChild(CaptureSetting->Widget);
		CaptureSetting->PopulateGUI();
    }
}

void USensorSettings::ApplyChanges()
{
	UJsonManager::SetStringField(Root, "id", ID);
	UJsonManager::SetStringField(Root, "type", Type);
	UJsonManager::SetBoolField(Root, "enabled", Enabled);
	if (CaptureInterval != -1.f) UJsonManager::SetNumberField(Root, "capture-interval", CaptureInterval);

	for (auto CaptureSetting : CaptureSettings)
	{
		CaptureSetting->ApplyChanges();
	}
}