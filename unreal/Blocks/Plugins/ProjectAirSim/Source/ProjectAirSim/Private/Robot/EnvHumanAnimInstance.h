#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"

#include "EnvHumanAnimInstance.generated.h"

UCLASS(config = Game)
class UEnvHumanAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UEnvHumanAnimInstance(const FObjectInitializer& ObjectInitializer)
        : UAnimInstance(ObjectInitializer) {}

    UFUNCTION(BlueprintImplementableEvent)
    void TickAnimationBPEvent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector velocity_;
};