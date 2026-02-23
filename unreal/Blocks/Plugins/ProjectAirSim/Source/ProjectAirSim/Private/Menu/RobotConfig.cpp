#include "Menu/RobotConfig.h"

void URobotConfig::Init(TSharedPtr<FJsonObject> RootIn, const FString& FileNameIn, USettingsMenu* MenuIn)
{
	this->Root = RootIn;
	this->FileName = FileNameIn;
	this->Menu = MenuIn;
	this->Widget = nullptr;

	if (!LoadSensors()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load sensors."));
	}
}

bool URobotConfig::LoadSensors()
{
	TArray<TSharedPtr<FJsonValue>> SensorsArray;
    if (!UJsonManager::GetObjectArray(Root, TEXT("sensors"), SensorsArray)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Robot does not contain sensors."));
		return false;
	}

	for (const auto& SensorValue : SensorsArray) 
	{
        TSharedPtr<FJsonObject> Sensor = SensorValue->AsObject();
		if (!Sensor.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Sensor."));
            continue;
        }
		USensorSettings* SensorSettings = NewObject<USensorSettings>(this);
		SensorSettings->Init(Sensor, Menu);
		Sensors.Add(SensorSettings);
	}
	return true;
}

void URobotConfig::PopulateGUI()
{
	Widget->FileNameText->SetText(FText::FromString(FileName));

	Widget->SensorSettingsContainer->ClearChildren();

	for (auto Sensor : Sensors) 
	{
		Sensor->Widget = CreateWidget<USensorSettingsWidget>(Widget, Widget->SensorSettingsWidgetClass);
		Sensor->Widget->Init(Sensor);
        Widget->SensorSettingsContainer->AddChild(Sensor->Widget);
		Sensor->PopulateGUI();
    }
}

void URobotConfig::ApplyChanges()
{
	for (auto Sensor : Sensors) 
	{
		Sensor->ApplyChanges();
	}

	UJsonManager::SaveJsonObject(Menu->ScriptFolderPath + Menu->ConfigFolder + FileName, Root);
}