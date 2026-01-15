#pragma once

#include "CoreMinimal.h"
#include "ActorSettingsWidget.generated.h"

class UActorSettings;

UCLASS()
class PROJECTAIRSIM_API UActorSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void Init(UActorSettings* OwnerIn);

	UFUNCTION()
	void CommitX(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitY(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitZ(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitRoll(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitPitch(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void CommitYaw(const FText& Text, ETextCommit::Type CommitType);

	UPROPERTY(meta = (BindWidget))
    UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* XTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* YTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* ZTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* RollTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* PitchTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* YawTextBox;

	UPROPERTY(meta = (BindWidget))
    UEditableTextBox* RobotConfigNameTextBox;

private:

	UActorSettings* Owner;
	
};