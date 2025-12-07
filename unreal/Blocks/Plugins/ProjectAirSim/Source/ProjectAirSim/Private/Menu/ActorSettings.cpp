#include "Menu/ActorSettings.h"
#include "Menu/SettingsMenu.h"

void UActorSettings::Init(TSharedPtr<FJsonObject> RootIn, USettingsMenu* MenuIn) 
{
	this->Root = RootIn;
	this->Menu = MenuIn;

	LoadAll();
}

bool UActorSettings::LoadAll() 
{
	if (!LoadName()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load name."));
		//return false;
	}
	if (!LoadXYZ())	
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load xyz."));
		//return false;
	}
	if (!LoadRPYDeg())	
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load rpy-deg."));
		//return false;
	}
	if (!LoadRobotConfig())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load robot-config."));
		//return false;
	}
	return true;
}

bool UActorSettings::LoadName() 
{
	FString NameValue;
	if (!UJsonManager::GetStringField(Root, "name", NameValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'name' field."));
		Name = "";
		return false;
	}

	Name = NameValue;
	return true;
}

bool UActorSettings::LoadXYZ()
{
  
	TSharedPtr<FJsonObject> Origin = Root->GetObjectField("origin");
	if (!Origin.IsValid()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'origin' field."));
		XYZ = FVector::ZeroVector;
		return false;
	} 

	FString XYZString;
	if (!UJsonManager::GetStringField(Origin, "xyz", XYZString)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'xyz' field."));
		XYZ = FVector::ZeroVector;
		return false;
	}
	  

	FVector XYZValue;
	if (!UJsonManager::ParseVectorString(XYZString, XYZValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid 'xyz' field."));
		XYZ = FVector::ZeroVector;
		return false;
	}		

	XYZ = XYZValue;
	return true;
}

bool UActorSettings::LoadRPYDeg()
{
  
	TSharedPtr<FJsonObject> Origin = Root->GetObjectField("origin");
	if (!Origin.IsValid()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'origin' field."));
		RPYDeg = FVector::ZeroVector;
		return false;
	} 

	FString RPYDegString;
	if (!UJsonManager::GetStringField(Origin, "rpy-deg", RPYDegString)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'rpy-deg' field."));
		RPYDeg = FVector::ZeroVector;
		return false;
	}
	  

	FVector RPYDegValue;
	if (!UJsonManager::ParseVectorString(RPYDegString, RPYDegValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid 'rpy-deg' field."));
		RPYDeg = FVector::ZeroVector;
		return false;
	}		

	RPYDeg = RPYDegValue;
	return true;
}

bool UActorSettings::LoadRobotConfig() 
{
	FString RobotConfigFileName;
	if (!UJsonManager::GetStringField(Root, "robot-config", RobotConfigFileName)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("No 'robot-config' field."));
		return false;
	}
	
	if (!Menu->LoadRobotConfig(RobotConfigFileName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load robot config file: %s"), *(RobotConfigFileName));
		return false;
	}

	RobotConfig = Menu->RobotConfigMap[RobotConfigFileName];
	return true;
}

void UActorSettings::PopulateGUI()
{
	Widget->NameText->SetText(FText::FromString(Name));
	
	Widget->XTextBox->SetText(FText::FromString(FString::SanitizeFloat(XYZ.X)));
	Widget->YTextBox->SetText(FText::FromString(FString::SanitizeFloat(XYZ.Y)));
	Widget->ZTextBox->SetText(FText::FromString(FString::SanitizeFloat(XYZ.Z)));

	Widget->RollTextBox->SetText(FText::FromString(FString::SanitizeFloat(RPYDeg.X)));
	Widget->PitchTextBox->SetText(FText::FromString(FString::SanitizeFloat(RPYDeg.Y)));
	Widget->YawTextBox->SetText(FText::FromString(FString::SanitizeFloat(RPYDeg.Z)));
	
	Widget->RobotConfigNameTextBox->SetText(FText::FromString(RobotConfig->FileName));

	RobotConfig->PopulateGUI();
}

void UActorSettings::ApplyChanges()
{
	UJsonManager::SetStringField(Root, "name", Name);
	
	TSharedPtr<FJsonObject> Origin = Root->GetObjectField("origin");

	UJsonManager::SetStringField(Origin, "xyz", UJsonManager::VectorToString(XYZ));
	UJsonManager::SetStringField(Origin, "rpy-deg", UJsonManager::VectorToString(RPYDeg));
	
	RobotConfig->ApplyChanges();
}