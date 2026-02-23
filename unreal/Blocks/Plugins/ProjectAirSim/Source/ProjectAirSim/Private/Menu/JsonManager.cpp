#include "Menu/JsonManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <cmath>

bool UJsonManager::LoadJsonFile(const FString& FilePath, FString& OutJsonString)
{
    return FFileHelper::LoadFileToString(OutJsonString, *FilePath);
}

bool UJsonManager::SaveJsonFile(const FString& FilePath, const FString& JsonString)
{
    return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool UJsonManager::LoadJsonObject(const FString& FilePath, TSharedPtr<FJsonObject>& OutObject)
{
    FString JsonString;
    if (!LoadJsonFile(FilePath, JsonString)) return false;
    JsonString = StripJsonComments(JsonString);
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    return FJsonSerializer::Deserialize(Reader, OutObject);
}

bool UJsonManager::SaveJsonObject(const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject)
{
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer)) return false;
    return SaveJsonFile(FilePath, JsonString);
}

bool UJsonManager::SetNumberField(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, double Value)
{
    if (!JsonObject.IsValid()) return false;
    JsonObject->SetNumberField(FieldName, Value);
    return true;
}

bool UJsonManager::GetNumberField(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, double& OutValue)
{
    if (!JsonObject.IsValid() || !JsonObject->HasTypedField<EJson::Number>(FieldName)) return false;
    OutValue = JsonObject->GetNumberField(FieldName);
    return true;
}

bool UJsonManager::ParseVectorString(const FString& VectorString, FVector& OutVector)
{
    TArray<FString> Parts;
    VectorString.ParseIntoArray(Parts, TEXT(" "), true);
    if (Parts.Num() != 3) return false;

    OutVector.X = FCString::Atod(*Parts[0]);
    OutVector.Y = FCString::Atod(*Parts[1]);
    OutVector.Z = FCString::Atod(*Parts[2]);
    return true;
}

FString UJsonManager::VectorToString(const FVector& Vector)
{
    return FString::Printf(TEXT("%f %f %f"), Vector.X, Vector.Y, Vector.Z);
}

bool UJsonManager::SetBoolField(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, bool Value)
{
    if (!JsonObject.IsValid()) return false;
    JsonObject->SetBoolField(FieldName, Value);
    return true;
}

bool UJsonManager::GetBoolField(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, bool& OutValue)
{
    if (!JsonObject.IsValid() || !JsonObject->HasTypedField<EJson::Boolean>(FieldName)) return false;
    OutValue = JsonObject->GetBoolField(FieldName);
    return true;
}

bool UJsonManager::SetStringField(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, const FString& Value)
{
    if (!JsonObject.IsValid()) return false;
    JsonObject->SetStringField(FieldName, Value);
    return true;
}

bool UJsonManager::GetStringField(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, FString& OutValue)
{
    if (!JsonObject.IsValid() || !JsonObject->HasTypedField<EJson::String>(FieldName)) return false;
    OutValue = JsonObject->GetStringField(FieldName);
    return true;
}

bool UJsonManager::GetObjectArray(TSharedPtr<FJsonObject> ParentObject, const FString& FieldName, TArray<TSharedPtr<FJsonValue>>& OutArray)
{
    if (!ParentObject.IsValid()) return false;
    const TArray<TSharedPtr<FJsonValue>>* ArrayPtr = nullptr;

    if (!ParentObject->TryGetArrayField(FieldName, ArrayPtr) || !ArrayPtr) {
        return false;
    }

    OutArray = *ArrayPtr;
    return true;
}

bool UJsonManager::ExtractSceneConfigFromPython(const FString& PythonFilePath, FString& OutSceneConfigName)
{
    FString PythonContent;
    if (!FFileHelper::LoadFileToString(PythonContent, *PythonFilePath)) {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load Python file: %s"), *PythonFilePath);
        return false;
    }

    FRegexPattern Pattern(TEXT("World\\s*\\(\\s*[^,]+,\\s*[\"']([^\"']+\\.jsonc)[\"']"));
    FRegexMatcher Matcher(Pattern, PythonContent);

    if (Matcher.FindNext()) {
        OutSceneConfigName = Matcher.GetCaptureGroup(1);
        UE_LOG(LogTemp, Log, TEXT("Found scene config in Python: %s"), *OutSceneConfigName);
        return true;
    }

    FRegexPattern AltPattern(TEXT("scene_config\\s*=\\s*[\"']([^\"']+\\.jsonc)[\"']"));
    FRegexMatcher AltMatcher(AltPattern, PythonContent);

    if (AltMatcher.FindNext()) {
        OutSceneConfigName = AltMatcher.GetCaptureGroup(1);
        UE_LOG(LogTemp, Log, TEXT("Found scene config in Python (alt pattern): %s"), *OutSceneConfigName);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("No scene config reference found in Python file"));
    return false;
}

FString UJsonManager::StripJsonComments(const FString& JsonString) {
    FString Result;
    Result.Reserve(JsonString.Len());

    bool bInString = false;
    bool bInSingleLineComment = false;
    bool bInMultiLineComment = false;

    for (int32 i = 0; i < JsonString.Len(); i++) {
        TCHAR Current = JsonString[i];
        TCHAR Next = (i + 1 < JsonString.Len()) ? JsonString[i + 1] : '\0';

        //Handle string literals
        if (!bInSingleLineComment && !bInMultiLineComment) {
            if (Current == '"' && (i == 0 || JsonString[i - 1] != '\\')) {
                bInString = !bInString;
            }
        }

        //Comment start
        if (!bInString && !bInSingleLineComment && !bInMultiLineComment) {
            if (Current == '/' && Next == '/') {
                bInSingleLineComment = true;
                i++;
                continue;
            } else if (Current == '/' && Next == '*') {
                bInMultiLineComment = true;
                i++;
                continue;
            }
        }

        //Comment end
        if (bInSingleLineComment) {
            if (Current == '\n' || Current == '\r') {
                bInSingleLineComment = false;
                Result.AppendChar(Current);
            }
            continue;
        }

        if (bInMultiLineComment) {
            if (Current == '*' && Next == '/') {
                bInMultiLineComment = false;
                i++;
            }
            continue;
        }

        Result.AppendChar(Current);
    }

    return Result;
}
