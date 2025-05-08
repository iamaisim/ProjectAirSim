// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "core_sim/message/message.hpp"
#include "core_sim/scene.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealSimTopicData.generated.h"

UCLASS()
class AUnrealSimTopicData : public AActor {
  GENERATED_BODY()

 public:
  explicit AUnrealSimTopicData(const FObjectInitializer& ObjectInitialize);

  void Tick(float DeltaTime) override;

  void SetTopicPublishedCallback(microsoft::projectairsim::Scene& Scene);

  template <typename TMessageClass>
  bool GetTopicData(const FString& TopicPath, TMessageClass& OutMessage);

  FString GetTopicDataAsJSON(const FString& TopicPath);

  UFUNCTION(BlueprintCallable, Category = "UnrealSimTopicData")
  static FString GetTopicDataAsJSONForBP(const FString& TopicPath);

  UFUNCTION(BlueprintCallable, Category = "UnrealSimTopicData")
  static FString GetValueFromJsonKey(FString Data, FString Key);

 protected:
  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  void OnTopicPublished(const std::string& topic_path,
                        const microsoft::projectairsim::MessageType& message_type,
                        const std::string& message);

  // key: topic path, value.first: message type, value.second: message buffer
  TMap<FString, TPair<microsoft::projectairsim::MessageType, std::string>>
      SimTopicDataMap;

  FCriticalSection UpdateMutex;

  // Example Unreal variables to save topic pose data to
  UPROPERTY(VisibleAnywhere)
  FVector Drone1Position = {0.f, 0.f, 0.f};
};
