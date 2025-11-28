// Copyright Epic Games, Inc. All Rights Reserved.

#include "Train/BaseVehicle.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
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

  // Initialize driver reference
  CurrentDriver = nullptr;
  CurrentRailSpline = nullptr;

  // AI controller class
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABaseVehicle::BeginPlay() {
  Super::BeginPlay();

  // Apply initial mesh transform
  ApplyVehicleMeshTransform();

  // Validate locomotion component
  if (!LocomotionComp) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: LocomotionComponent is null!"));
    return;
  }

  // Set initial rail spline if assigned
  if (CurrentRailSpline) {
    LocomotionComp->SetRailSpline(CurrentRailSpline);
  }
}

void ABaseVehicle::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Locomotion component handles movement logic
}

void ABaseVehicle::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (!PlayerInputComponent) {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: PlayerInputComponent is null!"));
    return;
  }

  // Bind input axes
  PlayerInputComponent->BindAxis("MoveForward", this,
                                 &ABaseVehicle::MoveForward);
  PlayerInputComponent->BindAxis("MoveBackward", this,
                                 &ABaseVehicle::MoveBackward);
  PlayerInputComponent->BindAxis("Brake", this, &ABaseVehicle::Brake);
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
    UE_LOG(LogTemp, Warning,
           TEXT("BaseVehicle: Already occupied by another player!"));
    return;
  }

  // Store driver reference
  CurrentDriver = Player;

  // Transfer control to this vehicle
  AController *PlayerController = Player->GetController();
  if (PlayerController) {
    PlayerController->Possess(this);
    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle: Player entered vehicle successfully"));
  } else {
    UE_LOG(LogTemp, Error, TEXT("BaseVehicle: Player has no controller!"));
    CurrentDriver = nullptr;
  }
}

void ABaseVehicle::ExitVehicle() {
  if (!IsOccupied()) {
    UE_LOG(LogTemp, Warning, TEXT("BaseVehicle: No player to exit!"));
    return;
  }

  if (!CurrentDriver) {
    UE_LOG(LogTemp, Error,
           TEXT("BaseVehicle: CurrentDriver is null but IsOccupied returned "
                "true!"));
    return;
  }

  // Return control to player character
  AController *PlayerController = GetController();
  if (PlayerController) {
    PlayerController->Possess(CurrentDriver);
    UE_LOG(LogTemp, Log,
           TEXT("BaseVehicle: Player exited vehicle successfully"));
  }

  CurrentDriver = nullptr;
}

void ABaseVehicle::MoveForward(float Value) {
  if (!LocomotionComp) {
    return;
  }

  if (FMath::Abs(Value) > 0.01f) {
    LocomotionComp->SetThrottle(Value);
  }
}

void ABaseVehicle::MoveBackward(float Value) {
  if (!LocomotionComp) {
    return;
  }

  if (FMath::Abs(Value) > 0.01f) {
    LocomotionComp->SetThrottle(-Value);
  }
}

void ABaseVehicle::Brake(float Value) {
  if (!LocomotionComp) {
    return;
  }

  if (FMath::Abs(Value) > 0.01f) {
    LocomotionComp->ApplyBrake(Value);
  }
}
