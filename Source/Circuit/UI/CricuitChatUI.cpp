// Fill out your copyright notice in the Description page of Project Settings.


#include "CricuitChatUI.h"
#include "../Player/CircuitPlayerController.h"
#include "Components/Border.h"
#include "Components/EditableText.h"
#include "CircuitChatMessageUI.h"
#include "Components/ScrollBox.h"

void UCricuitChatUI::SetChatInputActive(bool bActive)
{
	Border_ChatBackground->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, bActive ? 0.5f : 0.f));

	EditableText_Chat->SetVisibility(bActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCricuitChatUI::FocusChatInput()
{
	EditableText_Chat->SetKeyboardFocus();
}

void UCricuitChatUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EditableText_Chat->OnTextCommitted.AddDynamic(this, &UCricuitChatUI::HandleChatTextCommitted);
}

void UCricuitChatUI::HandleChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	const FString Message = Text.ToString().TrimStartAndEnd();

	EditableText_Chat->SetText(FText::GetEmpty());

	ACircuitPlayerController* PC = Cast<ACircuitPlayerController>(GetOwningPlayer());

	if (!IsValid(PC))
	{
		return;
	}



	if (!Message.IsEmpty())
	{
		PC->ServerSendChatMessage(Message);
	}

	PC->CloseChat();
}

void UCricuitChatUI::AddMessage(const FText& PlayerName, const FText& Message, const FLinearColor& NameColor)
{
	if (!ChatMessageClass||!ScrollBox_Chat)
	{
		return;
	}

	UCircuitChatMessageUI* MessageWidget = CreateWidget<UCircuitChatMessageUI>(GetOwningPlayer(), ChatMessageClass);

	if (!MessageWidget)
	{
		return;
	}

	MessageWidget->SetMessage(PlayerName, Message, NameColor);

	ScrollBox_Chat->AddChild(MessageWidget);
	
	const int32 MessageLimit = FMath::Max(1, MaxChatMessage);

	while (ScrollBox_Chat->GetChildrenCount() > MessageLimit)
	{
		ScrollBox_Chat->RemoveChildAt(0);
	}

	ScrollBox_Chat->ScrollToEnd();
}
