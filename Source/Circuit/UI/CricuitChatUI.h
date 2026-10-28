// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CricuitChatUI.generated.h"

class UBorder;
class UEditableText;
class UScrollBox;
class UCircuitChatMessageUI;

/**
 * 
 */
UCLASS()
class CIRCUIT_API UCricuitChatUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_ChatBackground;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText_Chat;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_Chat;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<UCircuitChatMessageUI> ChatMessageClass;

	UPROPERTY(EditDefaultsOnly, Category = "Chat", meta = (ClampMin = "1"))
	int32 MaxChatMessage = 100;

public:
	void SetChatInputActive(bool bActive);
	void FocusChatInput();

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

public:
	void AddMessage(const FText& PlayerName, const FText& Message, const FLinearColor& NameColor);


};
