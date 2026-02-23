#include "Menu/SceneConfig.h"
#include "Menu/SettingsMenu.h"

void USceneConfig::Init(TSharedPtr<FJsonObject> RootIn, const FString& FileNameIn, USettingsMenu* MenuIn) 
{
	this->FileName = FileNameIn;
	this->Menu = MenuIn;
	this->Root = RootIn;

	if (!LoadActors()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load actors."));
	}
}

bool USceneConfig::LoadActors()
{
	TArray<TSharedPtr<FJsonValue>> ActorsArray;
    if (!UJsonManager::GetObjectArray(Root, TEXT("actors"), ActorsArray)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Scene does not contain actors."));
		return false;
	}

	for (const auto& ActorValue : ActorsArray) 
	{
        TSharedPtr<FJsonObject> Actor = ActorValue->AsObject();
		if (!Actor.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Actor."));
            continue;
        }
		UActorSettings* ActorSettings = NewObject<UActorSettings>(this);
		ActorSettings->Init(Actor, Menu);
		Actors.Add(ActorSettings);
	}
	return true;
}

void USceneConfig::PopulateGUI() 
{
	if (Menu->SceneConfigFileNameText)
    {
        Menu->SceneConfigFileNameText->SetText(FText::FromString(FileName));
    }

	for (auto Actor : Actors) 
	{
		Actor->PopulateGUI();
    }
}

void USceneConfig::ApplyChanges()
{
	for (auto Actor : Actors) 
	{
		Actor->ApplyChanges();
    }

	UJsonManager::SaveJsonObject(Menu->ScriptFolderPath + Menu->ConfigFolder + FileName, Root);
}