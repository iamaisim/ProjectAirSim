#include "Menu/SettingsMenu.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"

static bool OpenFileDialogHelper(const FString& Title, const FString& initialPath, const FString& FileTypes, FString& OutPath);

void USettingsMenu::NativeConstruct()
{
    Super::NativeConstruct();

    ConfigFolder = "sim_config\\";
    ScriptFolderPath = "";
    PythonScriptName = "";
    VirtualEnvActivatePath = "";

    if (!IsDesignTime())
    {
        if (SetActivateButton) SetActivateButton->OnClicked.AddDynamic(this, &USettingsMenu::SetActivatePath);
        if (LoadScriptButton) LoadScriptButton->OnClicked.AddDynamic(this, &USettingsMenu::SelectPythonScript);
        if (RunScriptButton) RunScriptButton->OnClicked.AddDynamic(this, &USettingsMenu::RunScript);
    }
}

void USettingsMenu::BeginDestroy()
{
    if (FPlatformProcess::IsProcRunning(CurrentPythonProcess)) FPlatformProcess::TerminateProc(CurrentPythonProcess, true);
    Super::BeginDestroy();
}

void USettingsMenu::SetActivatePath()
{
    FString SelectedFile;
    if (!OpenFileDialogHelper(TEXT("Select Virtual Environment Activate Script"), FPaths::ProjectDir(), TEXT("All Files (*.*)|*.*"), SelectedFile))
    {
        UE_LOG(LogTemp, Warning, TEXT("No file selected for virtual environment activation script."));
        return;
    }
    VirtualEnvActivatePath = SelectedFile;
    LoadScriptButton->SetVisibility(ESlateVisibility::Visible);
}

void USettingsMenu::SelectPythonScript()
{
    FString SelectedFile;
    FString DefaultPath = FPaths::ProjectDir();
    if (!ScriptFolderPath.Equals("")) DefaultPath = ScriptFolderPath;
    if (OpenFileDialogHelper(TEXT("Select Python Script"), DefaultPath, TEXT("Python Scripts|*.py"), SelectedFile))
    {
        ScriptFolderPath = FPaths::GetPath(SelectedFile) + "\\";
        FString FileName = FPaths::GetCleanFilename(SelectedFile);
        OnFileSelected(FileName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Python script selected."));
    }
}

void USettingsMenu::RunScript()
{

    ApplyChanges();

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (!PlatformFile.FileExists(*(ScriptFolderPath + PythonScriptName)))
    {
        UE_LOG(LogTemp, Warning, TEXT("File not found: %s"), *(ScriptFolderPath + PythonScriptName));
        return;
    }

    FString Cmd = TEXT("C:\\Windows\\System32\\cmd.exe");
    
    FString Args = FString::Printf(
        TEXT("/K \"call %s && cd /d %s && python %s && cmd\""),
        *VirtualEnvActivatePath,
        *ScriptFolderPath,
        *PythonScriptName);
    
    if (FPlatformProcess::IsProcRunning(CurrentPythonProcess)) FPlatformProcess::TerminateProc(CurrentPythonProcess, true);

    CurrentPythonProcess = FPlatformProcess::CreateProc(
        *Cmd, 
        *Args, //params
        false, //bLaunchDetatched
        false, //bLaunchHidden
        false, //bLaunchReallyHidden
        nullptr, 0, nullptr, nullptr);
}

void USettingsMenu::OnFileSelected(const FString& FileName)
{
    if (!ValidateFile(FileName)) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not validate file: %s"), *(FileName));
        return;
    }

    PythonScriptName = FileName;
    
    LoadSelectedPythonScript();
}

bool USettingsMenu::ValidateFile(const FString& FileName)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (PlatformFile.FileExists(*(ScriptFolderPath + FileName)))
    {
        if (FileName.EndsWith(".py")) 
        {
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("File does not end in .py: %s"), *(FileName));
            return false;
        }
    }
    else 
    {
        UE_LOG(LogTemp, Warning, TEXT("File not found: %s"), *(ScriptFolderPath + FileName));
        return false;
    }
}

void USettingsMenu::LoadSelectedPythonScript()
{
    LoadedScriptOptionsContainer->SetVisibility(ESlateVisibility::Collapsed);

    FString SceneConfigFileName;
    if (!UJsonManager::ExtractSceneConfigFromPython(ScriptFolderPath + PythonScriptName, SceneConfigFileName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not extract scene config file."));
        return;
    }


    if (!LoadSceneConfig(SceneConfigFileName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load scene config file: %s"), *(SceneConfigFileName));
        return;
    }

    if (!PopulateGUI())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to populate GUI."));
        return;
    }

    LoadedScriptOptionsContainer->SetVisibility(ESlateVisibility::Visible);
}

bool USettingsMenu::LoadSceneConfig(const FString& FileName) 
{
    if (SceneConfigMap.Contains(FileName))
    {
        SceneConfig = SceneConfigMap[FileName];
    }
    else
    {
        TSharedPtr<FJsonObject> Root;
        if (!UJsonManager::LoadJsonObject(ScriptFolderPath + ConfigFolder + FileName, Root)) 
        {
        	UE_LOG(LogTemp, Warning, TEXT("Could not load Json: %s"), *(FileName));
        	return false;
        }
        SceneConfig = NewObject<USceneConfig>(this);
        SceneConfig->Init(Root, FileName, this);
        
        SceneConfigMap.Add(FileName, SceneConfig);
    }
    return true;
}

bool USettingsMenu::LoadRobotConfig(const FString& FileName)
{
    if (!RobotConfigMap.Contains(FileName))
    {
        TSharedPtr<FJsonObject> Root;
        if (!UJsonManager::LoadJsonObject(ScriptFolderPath + ConfigFolder + FileName, Root)) 
        {
        	UE_LOG(LogTemp, Warning, TEXT("Could not load Json: %s"), *(FileName));
        	return false;
        }
        URobotConfig* RobotConfig = NewObject<URobotConfig>(this);
        RobotConfig->Init(Root, FileName, this);
        
        RobotConfigMap.Add(FileName, RobotConfig);
    }
    return true;

}

bool USettingsMenu::PopulateGUI() 
{   
    if (PythonScriptNameText)
    {
        PythonScriptNameText->SetText(FText::FromString("Run " + PythonScriptName));
    }

    ActorSettingsContainer->ClearChildren();
    RobotConfigContainer->ClearChildren();

    for (int i = 0; i < SceneConfig->Actors.Num(); i++)
    {
        SceneConfig->Actors[i]->Widget = CreateWidget<UActorSettingsWidget>(this, ActorSettingsWidgetClass);
        SceneConfig->Actors[i]->Widget->Init(SceneConfig->Actors[i]);
        ActorSettingsContainer->AddChild(SceneConfig->Actors[i]->Widget);

        if (SceneConfig->Actors[i]->RobotConfig->Widget == nullptr)
        {
            SceneConfig->Actors[i]->RobotConfig->Widget = CreateWidget<URobotConfigWidget>(this, RobotConfigWidgetClass);
        }

        bool ContainsWidget = false;

        for (UWidget* Widget : RobotConfigContainer->GetAllChildren())
        {
            if (Widget == SceneConfig->Actors[i]->RobotConfig->Widget)
            {
                ContainsWidget = true;
                break;
            }
        }
        if (!ContainsWidget)
        {
            RobotConfigContainer->AddChild(SceneConfig->Actors[i]->RobotConfig->Widget);
        }
    }

    SceneConfig->PopulateGUI();


    return true;
}

void USettingsMenu::ApplyChanges()
{
    SceneConfig->ApplyChanges();
}

static bool OpenFileDialogHelper(const FString& Title, const FString& initialPath, const FString& FileTypes, FString& OutPath)
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        TArray<FString> OutFiles;
        const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
        
        bool bOpened = DesktopPlatform->OpenFileDialog(
            ParentWindowWindowHandle,
            Title,
            *initialPath,//default path
            TEXT(""),
            FileTypes,
            EFileDialogFlags::None,
            OutFiles
        );

        if (bOpened && OutFiles.Num() > 0)
        {
            OutPath = OutFiles[0];
            return true;
        }
    }
    return false;
}