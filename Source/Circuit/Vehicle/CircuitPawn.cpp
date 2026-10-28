// Copyright Epic Games, Inc. All Rights Reserved.

#include "CircuitPawn.h"
#include "Wheels/CircuitWheelFront.h"
#include "Wheels/CircuitWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "../Player/CircuitPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "../Game/CIRMatchGameState.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

void ACircuitPawn::Attack(const FInputActionValue& Value)
{
	if (!IsLocallyControlled() || !CanControlVehicle())
	{
		return;
	}

	ServerRPCAttack();
}

void ACircuitPawn::ServerRPCAttack_Implementation()
{
	if (!HasAuthority() || !IsValid(GetWorld()) || !CanControlVehicle())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	LastAttackTime = CurrentTime;

	const FVector Start = GetActorTransform().TransformPosition(AttackStartOffset);
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Vehicle);

	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		Start,
		End,
		ObjectParams,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	ACircuitPawn* HitVehicle = Cast<ACircuitPawn>(HitResult.GetActor());

	if (!IsValid(HitVehicle) || HitVehicle == this)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		HitVehicle,
		5.0f,
		GetController(),
		this,
		UDamageType::StaticClass()
	);

	MulticastPlayAttackHitEffect(HitResult.ImpactPoint);
}

float ACircuitPawn::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	ACircuitPawn* Attacker = Cast<ACircuitPawn>(DamageCauser);

	if (!IsValid(Attacker) || Attacker == this)
	{
		return 0.0f;
	}

	ACircuitPlayerState* VictimPS = GetPlayerState<ACircuitPlayerState>();

	ACircuitPlayerState* AttackerPS = Attacker->GetPlayerState<ACircuitPlayerState>();

	if (!IsValid(VictimPS) || !IsValid(AttackerPS) || VictimPS == AttackerPS)
	{
		return 0.0f;
	}

	constexpr int32 StealAmount = 5;

	if (!VictimPS->TrySpendCoins(StealAmount))
	{
		return 0.0f;
	}

	AttackerPS->AddCoins(StealAmount);

	return DamageAmount;
}

ACircuitPawn::ACircuitPawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void ACircuitPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACircuitPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACircuitPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACircuitPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACircuitPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACircuitPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACircuitPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACircuitPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACircuitPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACircuitPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACircuitPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACircuitPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACircuitPawn::ResetVehicle);

		//Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACircuitPawn::Attack);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACircuitPawn::BeginPlay()
{
	Super::BeginPlay();

	InitializeSpeedSetup();

	if (ACircuitPlayerState* PS = GetPlayerState<ACircuitPlayerState>())
	{
		HandleBonusSpeedChanged(PS->GetBonusMaxSpeed());
	}
}

void ACircuitPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	UpdateMatchControlState();

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

void ACircuitPawn::Steering(const FInputActionValue& Value)
{
	if (!CanControlVehicle())
	{
		ChaosVehicleMovement->SetSteeringInput(0.0f);
		return;
	}

	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACircuitPawn::Throttle(const FInputActionValue& Value)
{
	if (!CanControlVehicle())
	{
		ChaosVehicleMovement->SetThrottleInput(0.0f);
		return;
	}

	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetThrottleInput(GetLimitedThrottle(ThrottleValue));
}

void ACircuitPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void ACircuitPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACircuitPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACircuitPawn::StartHandbrake(const FInputActionValue& Value)
{
	if (!CanControlVehicle())
	{
		ChaosVehicleMovement->SetHandbrakeInput(true);
		return;
	}

	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACircuitPawn::StopHandbrake(const FInputActionValue& Value)
{
	if (!CanControlVehicle())
	{
		ChaosVehicleMovement->SetHandbrakeInput(true);
		return;
	}

	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACircuitPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACircuitPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACircuitPawn::ResetVehicle(const FInputActionValue& Value)
{
	if (!CanControlVehicle())
	{
		return;
	}

	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ACircuitPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	BindPlayerState();
}

void ACircuitPawn::UnPossessed()
{
	UnBindPlayerState();
	Super::UnPossessed();
}

void ACircuitPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BindPlayerState();
}

void ACircuitPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnBindPlayerState();
	Super::EndPlay(EndPlayReason);
}

void ACircuitPawn::BindPlayerState()
{
	ACircuitPlayerState* PS = GetPlayerState<ACircuitPlayerState>();

	if (!IsValid(PS))
	{
		UnBindPlayerState();
		return;
	}

	if (BoundPlayerState.Get() == PS && SpeedChangedHandle.IsValid())
	{
		return;
	}

	UnBindPlayerState();

	SpeedChangedHandle = PS->OnSpeedChanged.AddUObject(
		this,
		&ThisClass::HandleBonusSpeedChanged
	);

	BoundPlayerState = PS;

	HandleBonusSpeedChanged(PS->GetBonusMaxSpeed());
}

void ACircuitPawn::UnBindPlayerState()
{
	if (BoundPlayerState.IsValid() && SpeedChangedHandle.IsValid())
	{
		BoundPlayerState->OnSpeedChanged.Remove(SpeedChangedHandle);
	}

	SpeedChangedHandle.Reset();
	BoundPlayerState.Reset();
}

void ACircuitPawn::HandleBonusSpeedChanged(float NewBounsSpeed)
{
	CurrentMaxSpeedKmh = BaseMaxSpeedKmh + NewBounsSpeed;

	ApplyBonusSpeed(NewBounsSpeed);
}

float ACircuitPawn::GetLimitedThrottle(float RequestedThrottle) const
{
	if (!IsValid(ChaosVehicleMovement))
	{
		return RequestedThrottle;
	}

	const float CurrentSpeedKmh = ChaosVehicleMovement->GetForwardSpeedMPH() * 1.609344f;

	if (RequestedThrottle > 0.0f && CurrentSpeedKmh >= CurrentMaxSpeedKmh)
	{
		return 0.0f;
	}

	return RequestedThrottle;
}

void ACircuitPawn::InitializeSpeedSetup()
{
	if (bSpeedSetupInitialized || !IsValid(ChaosVehicleMovement))
	{
		return;
	}

	BaseEngineToque = ChaosVehicleMovement->EngineSetup.MaxTorque;
	BaseDragCoefficient = ChaosVehicleMovement->DragCoefficient;

	bSpeedSetupInitialized = true;
}

void ACircuitPawn::ApplyBonusSpeed(float NewBonusSpeed)
{
	InitializeSpeedSetup();

	if (!bSpeedSetupInitialized)
	{
		return;
	}

	const float BonusLevel = FMath::Max(0.0f, NewBonusSpeed) / 10.0f;
	const float TorqueMultiplier = 1.0f + BonusLevel * TorqueBonusPer10Speed;
	const float NewTorque = BaseEngineToque * TorqueMultiplier;
	const float DragMultiplier = FMath::Max(0.5f, 1.0f - BonusLevel * DragReductionPer10Speed);
	const float NewDrag = BaseDragCoefficient * DragMultiplier;

	ChaosVehicleMovement->SetMaxEngineTorque(NewTorque);
	ChaosVehicleMovement->SetDragCoefficient(NewDrag);

	UE_LOG(LogTemp, Warning,
		TEXT("Bonus: %.1f | Torque: %.1f | Drag: %.3f"),
		NewBonusSpeed,
		NewTorque,
		NewDrag
	);

}

bool ACircuitPawn::CanControlVehicle() const
{
	const ACIRMatchGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACIRMatchGameState>() : nullptr;

	if (!IsValid(GS))
	{
		return false;
	}

	return GS->GetMatchPhase() == ECIRMatchPhase::Playing;
}

void ACircuitPawn::UpdateMatchControlState()
{
	const ACIRMatchGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACIRMatchGameState>() : nullptr;

	if (!IsValid(GS))
	{
		if (!bMatchPhaseInitialized)
		{
			ApplyMatchPhase(ECIRMatchPhase::Waiting);
		}
		return;
	}

	const ECIRMatchPhase CurrentPhase = GS->GetMatchPhase();

	if (!bMatchPhaseInitialized || LastMatchPhase != CurrentPhase)
	{
		LastMatchPhase = CurrentPhase;
		bMatchPhaseInitialized = true;

		ApplyMatchPhase(CurrentPhase);
	}
}

void ACircuitPawn::ApplyMatchPhase(ECIRMatchPhase NewPhase)
{
	if (!IsValid(ChaosVehicleMovement))
	{
		return;
	}

	if (NewPhase == ECIRMatchPhase::Playing)
	{
		ChaosVehicleMovement->SetHandbrakeInput(false);
		return;
	}

	ChaosVehicleMovement->SetThrottleInput(0.0f);
	ChaosVehicleMovement->SetSteeringInput(0.0f);
	ChaosVehicleMovement->SetHandbrakeInput(true);
}

void ACircuitPawn::MulticastPlayAttackHitEffect_Implementation(
	FVector HitLocation)
{
	if (IsValid(AttackHitEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AttackHitEffect,
			HitLocation
		);
	}

	if (IsValid(AttackHitSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			AttackHitSound,
			HitLocation
		);
	}
}

#undef LOCTEXT_NAMESPACE