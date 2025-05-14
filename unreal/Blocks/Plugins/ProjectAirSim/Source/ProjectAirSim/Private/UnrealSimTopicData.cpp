// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "UnrealSimTopicData.h"

#include "Misc/ScopeLock.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "json.hpp"

namespace projectairsim = microsoft::projectairsim;

using json = nlohmann::json;

AUnrealSimTopicData::AUnrealSimTopicData(
    const FObjectInitializer& ObjectInitialize)
    : AActor(ObjectInitialize) {
  PrimaryActorTick.bCanEverTick = true;
}

void AUnrealSimTopicData::BeginPlay() { Super::BeginPlay(); }

void AUnrealSimTopicData::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // // Example of getting pose data through BP-callable static function
  // auto Drone1PositionJSON = AUnrealSimTopicData::GetTopicDataAsJSONForBP(
  //     "/Sim/SceneBasicDrone/robots/Drone1/actual_pose");

  // // Example of getting pose data and exposing it to Unreal variables
  // projectairsim::PoseStampedMessage PoseStampedMessageObj;
  // bool bFound = GetTopicData<projectairsim::PoseStampedMessage>(
  //     "/Sim/SceneBasicDrone/robots/Drone1/actual_pose", PoseStampedMessageObj);
  // if (bFound) {
  //   auto CurPositionNED = PoseStampedMessageObj.GetPosition();
  //   Drone1Position = UnrealTransform::NedToUnrealLinear(CurPositionNED);
  // }
}

void AUnrealSimTopicData::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
}

void AUnrealSimTopicData::SetTopicPublishedCallback(projectairsim::Scene& Scene) {
  Scene.SetCallbackTopicPublished(
      [this](const std::string& topic_path,
             const projectairsim::MessageType& message_type,
             const std::string& message) {
        this->AUnrealSimTopicData::OnTopicPublished(topic_path, message_type,
                                                    message);
      });
}

void AUnrealSimTopicData::OnTopicPublished(
    const std::string& topic_path, const projectairsim::MessageType& message_type,
    const std::string& message_buffer) {
  FString TopicPath(UTF8_TO_TCHAR(topic_path.c_str()));

  FScopeLock ScopeLock(&UpdateMutex);
  // Copy topic message data into map (expensive for images)
  SimTopicDataMap.Add(TopicPath, TPair<projectairsim::MessageType, std::string>(
                                     message_type, message_buffer));
}

template <typename TMessageClass>
bool AUnrealSimTopicData::GetTopicData(const FString& TopicPath,
                                       TMessageClass& OutMessage) {
  FScopeLock ScopeLock(&UpdateMutex);

  auto PairPtr = SimTopicDataMap.Find(TopicPath);
  if (!PairPtr || PairPtr->Key != OutMessage.GetType()) {
    return false;
  }

  OutMessage.Deserialize(PairPtr->Value);

  return true;
}

FString AUnrealSimTopicData::GetTopicDataAsJSON(const FString& TopicPath) {
  FScopeLock ScopeLock(&UpdateMutex);

  auto PairPtr = SimTopicDataMap.Find(TopicPath);
  if (!PairPtr) return "";

  const std::string& DataBuffer = PairPtr->Value;
  if (DataBuffer.length() == 0) return "";

  json ParsedData = json::from_msgpack(DataBuffer);

  FString DataJSON(ParsedData.dump().c_str());
  return DataJSON;
}

FString AUnrealSimTopicData::GetTopicDataAsJSONForBP(const FString& TopicPath) {
  AActor* ActorPtr = UGameplayStatics::GetActorOfClass(
      GEngine->GameViewport->GetWorld(), AUnrealSimTopicData::StaticClass());

  AUnrealSimTopicData* SimTopicDataActor = Cast<AUnrealSimTopicData>(ActorPtr);

  if (SimTopicDataActor == nullptr) return "";

  FString DataJSON = SimTopicDataActor->GetTopicDataAsJSON(TopicPath);
  return DataJSON;
}

FString AUnrealSimTopicData::GetValueFromJsonKey(FString Data, FString Key) {
  std::string DataStr = TCHAR_TO_UTF8(*Data);
  std::string KeyStr = TCHAR_TO_UTF8(*Key);
  auto ParsedData = json::parse(DataStr);

  FString Result(ParsedData[KeyStr].dump().c_str());
  return Result;
}
