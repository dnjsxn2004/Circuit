// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CircuitPlayerController.generated.h"

class UInputMappingContext;
class ACircuitPawn;
class UCircuitUI;
class UCricuitChatUI;
class UInputAction;
class UCircuitMatchWidget;

/**
 *  Vehicle Player Controller class
 *  Handles input mapping and user interface
 */
UCLASS()
class CIRCUIT_API ACircuitPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	/** If true, the optional steering wheel input mapping context will be registered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	bool bUseSteeringWheelControls = false;

	/** Optional Input Mapping Context to be used for steering wheel input.
	 *  This is added alongside the default Input Mapping Context and does not block other forms of input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta=(EditCondition="bUseSteeringWheelControls"))
	UInputMappingContext* SteeringWheelInputMappingContext;

	/** Pointer to the controlled vehicle pawn */
	UPROPERTY(Replicated)
	TObjectPtr<ACircuitPawn> VehiclePawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UCircuitUI> VehicleUIClass;

	/** Pointer to the UI widget */
	UPROPERTY()
	TObjectPtr<UCircuitUI> VehicleUI;

	

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:

	virtual void Tick(float Delta) override;

	// End Actor interface

	// Begin PlayerController interface
protected:

	virtual void OnPossess(APawn* InPawn) override;

	// End PlayerController interface

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCricuitChatUI> ChatUIClass;

	UPROPERTY()
	TObjectPtr<UCricuitChatUI> ChatUI;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> OpenChatAction;

	void OpenChat();

public:
	void CloseChat();

	void ReceiveChatMessage(const FText& PlayerName, const FText& Message, const FLinearColor& NameColor);

	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& PlayerName, const FString& Message, FLinearColor NameColor);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Match")
	TSubclassOf<UCircuitMatchWidget> MatchWidgetClass;

	UPROPERTY()
	TObjectPtr<UCircuitMatchWidget> MatchWidgetInstance;

};
