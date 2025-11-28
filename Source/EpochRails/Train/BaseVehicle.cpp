// Copyright Epic Games, Inc. All Rights Reserved.

#include "Train/BaseVehicle.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Train/LocomotionComponent.h"
#include "Train/RailSplineActor.h"

ABaseVehicle::ABaseVehicle() {
  PrimaryActorTick.bCanEverTick = true;

  // Create root component
  RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  SetRootComponent(RootComp);

  // Create vehicle mesh
  VehicleMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
  VehicleMesh->SetupAttachment(RootComp);
  VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));

  // Create camera spring arm
  SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
  SpringArm->SetupAttachment(RootComp);
  SpringArm->TargetArmLength = 600.0f;
  SpringArm->bUsePawnControlRotation = true;
  SpringArm->bEnableCameraLag = true;
  SpringArm->CameraLagSpeed = 3.0f;

  // Create camera
  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

  // Create locomotion component
  LocomotionComp =
      CreateDefaultSubobject<ULocomotionComponent>(TEXT("LocomotionComponent"));

  // Default transform settings
  VehicleMeshScale = FVector(1.0f, 1.0f, 1.0f);
  VehicleMeshRotation = FRotator(0.0f, 0.0f, 0.0f);

  // Initialize references
  CurrentDriver = nullptr;
  CurrentRailSpline = nullptr;

  // Input settings
  InputMappingPriority = 1;

  // AI controller
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABaseVehicle::BeginPlay() {
  Super::BeginPlay();

  ApplyVehicleMeshTransform();

  if (!LocomotionComp) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: LocomotionComponent is null!"));
    return;
  }

  if (CurrentRailSpline) {
    LocomotionComp->SetRailSpline(CurrentRailSpline);
  }
}

void ABaseVehicle::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABaseVehicle::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (!PlayerInputComponent) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: PlayerInputComponent is null!"));
    return;
  }

  // Cast to Enhanced Input Component
  UEnhancedInputComponent *EnhancedInputComponent =
      Cast<UEnhancedInputComponent>(PlayerInputComponent);
  if (!EnhancedInputComponent) {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle: Failed to cast to EnhancedInputComponent!"));
    return;
  }

  // Bind Enhanced Input Actions
  if (ThrottleAction) {
    EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered,
                                       this, &ABaseVehicle::OnThrottle);
    EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed,
                                       this, &ABaseVehicle::OnThrottle);
  }

  if (BrakeAction) {
    EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered,
                                       this, &ABaseVehicle::OnBrake);
    EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed,
                                       this, &ABaseVehicle::OnBrake);
  }

  if (ExitAction) {
    EnhancedInputComponent->BindAction(ExitAction, ETriggerEvent::Started, this,
                                       &ABaseVehicle::OnExitVehicle);
  }

  UE_LOG(LogTemp, Log,
         TEXT("BaseVehicle: Enhanced Input bindings set up successfully"));
}

void ABaseVehicle::ApplyVehicleMeshTransform() {
  if (!VehicleMesh) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: VehicleMesh is null!"));
    return;
  }

  VehicleMesh->SetRelativeScale3D(VehicleMeshScale);
  VehicleMesh->SetRelativeRotation(VehicleMeshRotation);

  UE_LOG(LogTemp, Log,
         TEXT("BaseVehicle: Applied mesh transform - Scale: %s, Rotation: %s"),
         *VehicleMeshScale.ToString(), *VehicleMeshRotation.ToString());
}

void ABaseVehicle::EnterVehicle(APawn *Player) {
  if (!Player) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: Attempted to enter with null player!"));
    return;
  }

  if (IsOccupied()) {
    UE_LOG(LogTemp, Warning, TEXT("BaseVehicle: Already occupied!"));
    return;
  }

  CurrentDriver = Player;

  AController *PlayerController = Player->GetController();
  if (!PlayerController) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: Player has no controller!"));
    CurrentDriver = nullptr;
    return;
  }

  // Possess vehicle
  PlayerController->Possess(this);

  // Add input mapping context
  AddInputMappingContext();

  UE_LOG(LogTemp, Log,
         TEXT("BaseVehicle: Player entered vehicle successfully"));
}

void ABaseVehicle::ExitVehicle() {
  if (!IsOccupied()) {
    UE_LOG(LogTemp, Warning, TEXT("BaseVehicle: No player to exit!"));
    return;
  }

  if (!CurrentDriver) {
    return;
  }

  AController *PlayerController = GetController();
  if (PlayerController) {
    // Remove input mapping context
    RemoveInputMappingContext();

    // Return control to player
    PlayerController->Possess(CurrentDriver);

    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle: Player exited vehicle successfully"));
  }

  CurrentDriver = nullptr;
}

void ABaseVehicle::OnThrottle(const FInputActionValue &Value) {
  if (!LocomotionComp) {
    return;
  }

  const float ThrottleValue = Value.Get<float>();
  LocomotionComp->SetThrottle(ThrottleValue);
}

void ABaseVehicle::OnBrake(const FInputActionValue &Value) {
  if (!LocomotionComp) {
    return;
  }

  const float BrakeValue = Value.Get<float>();
  LocomotionComp->ApplyBrake(BrakeValue);
}

void ABaseVehicle::OnExitVehicle(const FInputActionValue &Value) {
  ExitVehicle();
}

void ABaseVehicle::AddInputMappingContext() {
  if (!VehicleMappingContext) {
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: VehicleMappingContext not assigned!"));
    return;
  }

  APlayerController *PlayerController =
      Cast<APlayerController>(GetController());
  if (!PlayerController) {
    return;
  }

  UEnhancedInputLocalPlayerSubsystem *Subsystem =
      ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
          PlayerController->GetLocalPlayer());
  if (Subsystem) {
    Subsystem->AddMappingContext(VehicleMappingContext, InputMappingPriority);
    UE_LOG(LogTemp, Log, TEXT("BaseVehicle: Input mapping context added"));
  }
}

void ABaseVehicle::RemoveInputMappingContext() {
  if (!VehicleMappingContext) {
    return;
  }

  APlayerController *PlayerController =
      Cast<APlayerController>(GetController());
  if (!PlayerController) {
    return;
  }

  UEnhancedInputLocalPlayerSubsystem *Subsystem =
      ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
          PlayerController->GetLocalPlayer());
  if (Subsystem) {
    Subsystem->RemoveMappingContext(VehicleMappingContext);
    UE_LOG(LogTemp, Log, TEXT("BaseVehicle: Input mapping context removed"));
  }
}
