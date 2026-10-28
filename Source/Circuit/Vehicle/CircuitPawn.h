// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "../CIRMatchTypes.h"
#include "CircuitPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
class ACircuitPlayerState;
class UNiagaraSystem;
class USoundBase;

struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateVehicle, Log, All);

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract)
class ACircuitPawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	/** Cast pointer to the Chaos Vehicle movement component */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:

	/** Steering Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ResetVehicleAction;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle | Attack")
	float AttackRange = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle | Attack")
	FVector AttackStartOffset = FVector(200.0f, 0.0f, 50.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle | Attack")
	float AttackCooldown = 0.5f;

	float LastAttackTime = -1000.0f;

	void Attack(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void ServerRPCAttack();

	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Effect")
	TObjectPtr<UNiagaraSystem> AttackHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Effect")
	TObjectPtr<USoundBase> AttackHitSound;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackHitEffect(FVector HitLocation);


public:
	ACircuitPawn();

	// Begin Pawn interface

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// End Pawn interface

	// Begin Actor interface
	virtual void BeginPlay() override;

	virtual void Tick(float Delta) override;

	// End Actor interface

protected:

	/** Handles steering input */
	void Steering(const FInputActionValue& Value);

	/** Handles throttle input */
	void Throttle(const FInputActionValue& Value);

	/** Handles brake input */
	void Brake(const FInputActionValue& Value);

	/** Handles brake start/stop inputs */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Handles handbrake start/stop inputs */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Handles look around input */
	void LookAround(const FInputActionValue& Value);

	/** Handles toggle camera input */
	void ToggleCamera(const FInputActionValue& Value);

	/** Handles reset vehicle input */
	void ResetVehicle(const FInputActionValue& Value);

	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

public:
	/** Returns the front spring arm subobject */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Returns the front camera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Returns the back spring arm subobject */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Returns the back camera subobject */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void BindPlayerState();
	void UnBindPlayerState();
	
	void HandleBonusSpeedChanged(float NewBounsSpeed);

private:
	TWeakObjectPtr<ACircuitPlayerState> BoundPlayerState;

	FDelegateHandle SpeedChangedHandle;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Speed")
	float BaseMaxSpeedKmh = 100.0f;

	float CurrentMaxSpeedKmh = 100.0f;

	float GetLimitedThrottle(float RequstedThrottle) const;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Speed")
	float TorqueBonusPer10Speed = 0.10f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Speed")
	float DragReductionPer10Speed = 0.05f;

private:
	float BaseEngineToque = 0.0f;
	float BaseDragCoefficient = 0.0f;

	bool bSpeedSetupInitialized = false;

	void InitializeSpeedSetup();
	void ApplyBonusSpeed(float NewBonusSpeed);

	bool CanControlVehicle() const;
	bool bMatchPhaseInitialized = false;

	void UpdateMatchControlState();
	void ApplyMatchPhase(ECIRMatchPhase NewPhase);

	ECIRMatchPhase LastMatchPhase = ECIRMatchPhase::Waiting;
};
