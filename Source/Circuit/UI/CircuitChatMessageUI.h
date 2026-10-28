// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CircuitChatMessageUI.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class CIRCUIT_API UCircuitChatMessageUI : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Name;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Message;

public:
	void SetMessage(const FText& PlayerName, const FText& Message, const FLinearColor& NameColor);
};
