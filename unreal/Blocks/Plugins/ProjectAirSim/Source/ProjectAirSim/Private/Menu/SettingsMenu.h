#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Menu/JsonManager.h"
#include "Menu/SceneConfig.h"
#include "Menu/RobotConfig.h"
#include "Menu/ActorSettingsWidget.h"
#include "Menu/RobotConfigWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"

#include "SettingsMenu.generated.h"

UCLASS()
class PROJECTAIRSIM_API USettingsMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	virtual void BeginDestroy() override;

	void OnFileSelected(const FString& FileName);

	bool ValidateFile(const FString& FileName);

	bool LoadSceneConfig(const FString& FileName);

	bool LoadRobotConfig(const FString& FileName);
	
	bool PopulateGUI();

	UFUNCTION()
	void SetActivatePath();

	UFUNCTION()
	void SelectPythonScript();

	UFUNCTION()
	void LoadSelectedPythonScript();

	UFUNCTION()
	void ApplyChanges();

	UFUNCTION()
	void CloseMenu();
	
	FString ScriptFolderPath;
	FString ConfigFolder;

	UPROPERTY()
	TMap<FString, USceneConfig*> SceneConfigMap;
	UPROPERTY()
	TMap<FString, URobotConfig*> RobotConfigMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actors")
	TSubclassOf<UActorSettingsWidget> ActorSettingsWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Robot Config")
	TSubclassOf<URobotConfigWidget> RobotConfigWidgetClass;

	UPROPERTY(meta = (BindWidget))
    UTextBlock* SceneConfigFileNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PythonScriptNameText;

	UPROPERTY()
	USceneConfig* SceneConfig;

private:

	FProcHandle CurrentPythonProcess;
	
    
	FString PythonScriptName;
	FString VirtualEnvActivatePath;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* LoadedScriptOptionsContainer;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* ActorSettingsContainer;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* RobotConfigContainer;

	UPROPERTY(meta = (BindWidget))
	UButton* SetActivateButton;

	UPROPERTY(meta = (BindWidget))
	UButton* LoadScriptButton;

	UPROPERTY(meta = (BindWidget))
    UButton* RunScriptButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UFUNCTION()
    void RunScript();
};
