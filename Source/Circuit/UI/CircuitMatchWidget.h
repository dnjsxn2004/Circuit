// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CircuitMatchWidget.generated.h"

class UTextBlock;
class ACircuitPlayerState;
class ACIRMatchGameState;
class UBorder;
/**
 * 
 */
UCLASS()
class CIRCUIT_API UCircuitMatchWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CoinText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> SpeedText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> MatchPhaseText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> RemainingTimeText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> ResultPanel;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ResultText;

private:
    void UpdateCoins(int32 NewCoins);
    void UpdateSpeed(float NewBonusSpeed);
    void UpdateMatchData();

    void BindGameData();
    void UnbindGameData();

    TWeakObjectPtr<ACircuitPlayerState> BoundPlayerState;
    TWeakObjectPtr<ACIRMatchGameState> BoundGameState;

    FDelegateHandle CoinsChangedHandle;
    FDelegateHandle SpeedChangedHandle;
    FDelegateHandle MatchDataChangedHandle;

    FTimerHandle BindRetryTimerHandle;

    void RetryBindGameData();
    void UpdateMatchResult();
};
