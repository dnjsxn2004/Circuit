// Fill out your copyright notice in the Description page of Project Settings.


#include "../UI/CircuitChatMessageUI.h"
#include "Components/TextBlock.h"

void UCircuitChatMessageUI::SetMessage(const FText& PlayerName, const FText& Message, const FLinearColor& NameColor)
{
	TextBlock_Name->SetText(PlayerName);
	TextBlock_Name->SetColorAndOpacity(FSlateColor(NameColor));
	TextBlock_Message->SetText(Message);
}
