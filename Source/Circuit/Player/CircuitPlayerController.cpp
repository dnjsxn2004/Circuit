// Copyright Epic Games, Inc. All Rights Reserved.


#include "CircuitPlayerController.h"
#include "../Vehicle/CircuitPawn.h"
#include "../UI/CircuitUI.h"
#include "../UI/CricuitChatUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "Components/EditableText.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "CircuitPlayerState.h"
#include "../UI/CircuitMatchWidget.h"

void ACircuitPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, VehiclePawn);
}

void ACircuitPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬에만 ui 생성
	if (!IsLocalController())
	{
		return;
	}
	
	if (VehicleUIClass)
	{
		VehicleUI = CreateWidget<UCircuitUI>(this, VehicleUIClass);

		if (IsValid(VehicleUI))
		{
			VehicleUI->AddToViewport();
		}
	}

	if (ChatUIClass)
	{
		ChatUI = CreateWidget<UCricuitChatUI>(this, ChatUIClass);

		if (ChatUI)
		{
			ChatUI->AddToViewport();
			ChatUI->SetChatInputActive(false);
		}
	}

	if (MatchWidgetClass)
	{
		MatchWidgetInstance = CreateWidget<UCircuitMatchWidget>(
			this,
			MatchWidgetClass
		);

		if (IsValid(MatchWidgetInstance))
		{
			MatchWidgetInstance->AddToViewport();
		}
	}
}

void ACircuitPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		// optionally add the steering wheel context
		if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (OpenChatAction)
		{
			EnhancedInput->BindAction(
				OpenChatAction,
				ETriggerEvent::Started,
				this,
				&ACircuitPlayerController::OpenChat
			);
		}
	}
}

void ACircuitPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void ACircuitPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<ACircuitPawn>(InPawn);
}

void ACircuitPlayerController::OpenChat()
{
	if (!IsValid(ChatUI))
	{
		return;
	}

	ChatUI->SetChatInputActive(true);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);

	ChatUI->FocusChatInput();
}

void ACircuitPlayerController::CloseChat()
{
	if (!IsValid(ChatUI))
	{
		return;
	}

	ChatUI->SetChatInputActive(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ACircuitPlayerController::ReceiveChatMessage(const FText& PlayerName, const FText& Message, const FLinearColor& NameColor)
{
	if (!IsValid(ChatUI))
	{
		return;
	}

	ChatUI->AddMessage(PlayerName, Message, NameColor);
}

void ACircuitPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	const FString CleanMessage = Message.TrimStartAndEnd();

	if (CleanMessage.IsEmpty())
	{
		return;
	}

	const ACircuitPlayerState* SenderState = GetPlayerState<ACircuitPlayerState>();

	if (!IsValid(SenderState))
	{
		return;
	}

	static const FLinearColor ChatColors[4] =
	{
		FLinearColor(0.2f,0.7f, 1.0f, 1.0f),
		FLinearColor(1.0f, 0.5f, 0.1f, 1.0f),
		FLinearColor(0.5f, 1.0f, 0.2f, 1.0f),
		FLinearColor(0.8f, 0.4f, 1.0f, 1.0f)

	};

	const int32 Slot = SenderState->GetChatColorSlot();

	if (Slot < 0 || Slot >=4)
	{
		return;
	}

	FString PlayerName = SenderState->GetPlayerName();

	if (PlayerName.IsEmpty())
	{
		PlayerName = TEXT("플레이어");
	}

	const FLinearColor NameColor = ChatColors[Slot];

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACircuitPlayerController* TargetPC = Cast<ACircuitPlayerController>(It->Get());

		if (IsValid(TargetPC))
		{
			TargetPC->ClientReceiveChatMessage(PlayerName, CleanMessage, NameColor);
		}
	}
}

void ACircuitPlayerController::ClientReceiveChatMessage_Implementation(const FString& PlayerName,const FString& Message, FLinearColor NameColor)
{
	ReceiveChatMessage(FText::FromString(PlayerName), FText::FromString(Message), NameColor);
}
