#include "Menu/SettingsMenu.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
    #include "Windows/AllowWindowsPlatformTypes.h"
    #include "Windows/PreWindowsApi.h"
    #include <shobjidl.h>
    #include "Windows/PostWindowsApi.h"
    #include "Windows/HideWindowsPlatformTypes.h"

    static bool RunNativeWindowsFileDialog(const FString& Title, const FString& DefaultPath, const FString& Filter, FString& OutFile);
#endif


void USettingsMenu::NativeConstruct()
{
    Super::NativeConstruct();

    ConfigFolder = "sim_config/";
    ScriptFolderPath = "";
    PythonScriptName = "";
    VirtualEnvActivatePath = "";

    if (!IsDesignTime())
    {
        if (SetActivateButton) SetActivateButton->OnClicked.AddDynamic(this, &USettingsMenu::SetActivatePath);
        if (LoadScriptButton) LoadScriptButton->OnClicked.AddDynamic(this, &USettingsMenu::SelectPythonScript);
        if (RunScriptButton) RunScriptButton->OnClicked.AddDynamic(this, &USettingsMenu::RunScript);
        if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &USettingsMenu::CloseMenu);
    }
}

void USettingsMenu::CloseMenu() 
{
    RemoveFromParent();
}

void USettingsMenu::BeginDestroy()
{
    if (FPlatformProcess::IsProcRunning(CurrentPythonProcess)) FPlatformProcess::TerminateProc(CurrentPythonProcess, true);
    Super::BeginDestroy();
}

void USettingsMenu::SetActivatePath()
{
    FString SelectedFile;
    FString DefaultPath = FPaths::ConvertRelativePathToFull(FPlatformProcess::UserDir());
   
#if PLATFORM_WINDOWS
    if (RunNativeWindowsFileDialog(TEXT("Select Virtual Environment Activate Script"), DefaultPath, TEXT("All Files|*"), SelectedFile))
    {
        VirtualEnvActivatePath = SelectedFile;
        LoadScriptButton->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("File selection cancelled or failed."));
    }
#endif
}

void USettingsMenu::SelectPythonScript()
{
    FString SelectedFile;
    FString DefaultPath = ScriptFolderPath.IsEmpty() ? FPaths::ProjectDir() : ScriptFolderPath;

#if PLATFORM_WINDOWS
    if (RunNativeWindowsFileDialog(TEXT("Select Python Script"), DefaultPath, TEXT("Python Scripts|*.py"), SelectedFile))
    {
        ScriptFolderPath = FPaths::GetPath(SelectedFile) + TEXT("\\");
        FString FileName = FPaths::GetCleanFilename(SelectedFile);
        OnFileSelected(FileName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Python script selected."));
    }
#endif
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
    FString Cmd;
    FString Args;
#if PLATFORM_WINDOWS
    Cmd = TEXT("C:\\Windows\\System32\\cmd.exe");
    Args = FString::Printf(
        TEXT("/K \"call %s && cd /d %s && python %s && cmd\""),
        *VirtualEnvActivatePath,
        *ScriptFolderPath,
        *PythonScriptName);
#elif PLATFORM_LINUX
    Cmd = TEXT("/bin/bash");
    Args = FString::Printf(
        TEXT("-c \"source '%s' && cd '%s' && python3 '%s'; exec bash\""),
        *VirtualEnvActivatePath,
        *ScriptFolderPath,
        *PythonScriptName);
#endif

    if (FPlatformProcess::IsProcRunning(CurrentPythonProcess)) FPlatformProcess::TerminateProc(CurrentPythonProcess, true);

    CurrentPythonProcess = FPlatformProcess::CreateProc( *Cmd, *Args, false, false, false, nullptr, 0, nullptr, nullptr);
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

#if PLATFORM_WINDOWS
static bool RunNativeWindowsFileDialog(const FString& Title, const FString& DefaultPath, const FString& Filter, FString& OutFile)
{
    IFileOpenDialog* FileDialog;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&FileDialog));

    if (FAILED(hr)) return false;

    FileDialog->SetTitle(*Title);

    IShellItem* DefaultFolderItem;
    FString WindowsPath = DefaultPath.Replace(TEXT("/"), TEXT("\\"));
    if (SUCCEEDED(SHCreateItemFromParsingName(*WindowsPath, NULL, IID_PPV_ARGS(&DefaultFolderItem))))
    {
        FileDialog->SetFolder(DefaultFolderItem);
        DefaultFolderItem->Release();
    }

    FString FilterName, FilterSpec;
    if (Filter.Split(TEXT("|"), &FilterName, &FilterSpec))
    {
        COMDLG_FILTERSPEC FileTypes[] = { { *FilterName, *FilterSpec } };
        FileDialog->SetFileTypes(1, FileTypes);
    }
    else
    {
        COMDLG_FILTERSPEC FileTypes[] = { { L"All Files", L"*.*" } };
        FileDialog->SetFileTypes(1, FileTypes);
    }

    hr = FileDialog->Show(NULL);

    bool bSuccess = false;
    if (SUCCEEDED(hr))
    {
        IShellItem* Item;
        if (SUCCEEDED(FileDialog->GetResult(&Item)))
        {
            PWSTR FilePath;
            if (SUCCEEDED(Item->GetDisplayName(SIGDN_FILESYSPATH, &FilePath)))
            {
                OutFile = FString(FilePath);
                CoTaskMemFree(FilePath);
                bSuccess = true;
            }
            Item->Release();
        }
    }
    FileDialog->Release();
    return bSuccess;
}
#endif