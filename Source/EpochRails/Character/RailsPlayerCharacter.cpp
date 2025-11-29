#include "Character/RailsPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Interaction/InteractableInterface.h"
#include "Train/BaseVehicle.h"
#include "Utils/AimTraceService.h"

DEFINE_LOG_CATEGORY_STATIC(LogRailsChar, Log, All);

//==================== CONSTRUCTOR ====================//

ARailsPlayerCharacter::ARailsPlayerCharacter() {
  PrimaryActorTick.bCanEverTick = true;

  // ========== Character Movement Setup ==========
  bUseControllerRotationYaw = true;
  bUseControllerRotationPitch = false;
  bUseControllerRotationRoll = false;

  if (UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    MoveComp->bOrientRotationToMovement = false;
    MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
    MoveComp->MaxWalkSpeed = WalkSpeed;
    MoveComp->JumpZVelocity = 400.f;
    MoveComp->AirControl = 0.2f;
  }

  // ========== Full Body Mesh Setup ==========
  GetMesh()->SetOwnerNoSee(false);
  GetMesh()->bCastHiddenShadow = true;

  // ========== Camera Setup ==========
  // Camera is now created in Blueprint - set pointer to null
  // You must add Camera Component in Blueprint and name it "FirstPersonCamera"
  FirstPersonCamera = nullptr;

  // ========== Input Mapping Context ==========
  static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCObj(
      TEXT("/Game/Input/IMC_RailsDefault"));
  if (IMCObj.Succeeded()) {
    DefaultMappingContext = IMCObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IMC_RailsDefault"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IMC_RailsDefault"));
  }

  // ========== Input Actions ==========

  // Move Action
  static ConstructorHelpers::FObjectFinder<UInputAction> IA_MoveObj(
      TEXT("/Game/Input/Actions/IA_Move"));
  if (IA_MoveObj.Succeeded()) {
    MoveAction = IA_MoveObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IA_Move"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IA_Move"));
  }

  // Look Action
  static ConstructorHelpers::FObjectFinder<UInputAction> IA_LookObj(
      TEXT("/Game/Input/Actions/IA_Look"));
  if (IA_LookObj.Succeeded()) {
    LookAction = IA_LookObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IA_Look"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IA_Look"));
  }

  // Interact Action
  static ConstructorHelpers::FObjectFinder<UInputAction> IA_InteractObj(
      TEXT("/Game/Input/Actions/IA_Interact"));
  if (IA_InteractObj.Succeeded()) {
    InteractAction = IA_InteractObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IA_Interact"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IA_Interact"));
  }

  // Toggle Build Mode Action
  static ConstructorHelpers::FObjectFinder<UInputAction> IA_BuildObj(
      TEXT("/Game/Input/Actions/IA_ToggleBuildMode"));
  if (IA_BuildObj.Succeeded()) {
    ToggleBuildModeAction = IA_BuildObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IA_ToggleBuildMode"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IA_ToggleBuildMode"));
  }

  // Exit Mode Action
  static ConstructorHelpers::FObjectFinder<UInputAction> IA_ExitObj(
      TEXT("/Game/Input/Actions/IA_ExitMode"));
  if (IA_ExitObj.Succeeded()) {
    ExitModeAction = IA_ExitObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IA_ExitMode"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IA_ExitMode"));
  }

  // Sprint Action
  static ConstructorHelpers::FObjectFinder<UInputAction> IA_SprintObj(
      TEXT("/Game/Input/Actions/IA_Sprint"));
  if (IA_SprintObj.Succeeded()) {
    SprintAction = IA_SprintObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IA_Sprint"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IA_Sprint"));
  }

  // Jump Action
  static ConstructorHelpers::FObjectFinder<UInputAction> IA_JumpObj(
      TEXT("/Game/Input/Actions/IA_Jump"));
  if (IA_JumpObj.Succeeded()) {
    JumpAction = IA_JumpObj.Object;
    UE_LOG(LogRailsChar, Log, TEXT("Successfully loaded IA_Jump"));
  } else {
    UE_LOG(LogRailsChar, Error, TEXT("Failed to load IA_Jump"));
  }

  // ========== Initial State ==========
  HandState = EHandState::Empty;
  CurrentHeldItem = nullptr;
  bIsSprinting = false;
}

//==================== BEGIN PLAY ====================//

void ARailsPlayerCharacter::BeginPlay() {
  Super::BeginPlay();

  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer())) {
      if (DefaultMappingContext) {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
        UE_LOG(LogRailsChar, Log, TEXT("Added Input Mapping Context"));
      } else {
        UE_LOG(LogRailsChar, Error, TEXT("DefaultMappingContext is null!"));
      }
    }
  }

  SetPlayerMode(EPlayerMode::Walking);
  HideHeadForOwner();

  if (UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    MoveComp->MaxWalkSpeed = WalkSpeed;
  }

  // Validate camera component
  if (!FirstPersonCamera) {
    UE_LOG(LogRailsChar, Warning,
           TEXT("FirstPersonCamera is null! Make sure to add Camera Component "
                "in Blueprint and name it 'FirstPersonCamera'."));
  } else {
    UE_LOG(LogRailsChar, Log, TEXT("FirstPersonCamera found: %s"),
           *FirstPersonCamera->GetName());
  }
}

//==================== SETUP INPUT ====================//

void ARailsPlayerCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent *EIC =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

    // Move
    if (MoveAction) {
      EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this,
                      &ARailsPlayerCharacter::Move);
    } else {
      UE_LOG(LogRailsChar, Error, TEXT("MoveAction is null"));
    }

    // Look
    if (LookAction) {
      EIC->BindAction(LookAction, ETriggerEvent::Triggered, this,
                      &ARailsPlayerCharacter::Look);
    } else {
      UE_LOG(LogRailsChar, Error, TEXT("LookAction is null"));
    }

    // Interact
    if (InteractAction) {
      EIC->BindAction(InteractAction, ETriggerEvent::Started, this,
                      &ARailsPlayerCharacter::Interact);
    } else {
      UE_LOG(LogRailsChar, Error, TEXT("InteractAction is null"));
    }

    // Toggle Build Mode
    if (ToggleBuildModeAction) {
      EIC->BindAction(ToggleBuildModeAction, ETriggerEvent::Started, this,
                      &ARailsPlayerCharacter::ToggleBuildMode);
    } else {
      UE_LOG(LogRailsChar, Error, TEXT("ToggleBuildModeAction is null"));
    }

    // Exit Mode
    if (ExitModeAction) {
      EIC->BindAction(ExitModeAction, ETriggerEvent::Started, this,
                      &ARailsPlayerCharacter::ExitCurrentMode);
    } else {
      UE_LOG(LogRailsChar, Error, TEXT("ExitModeAction is null"));
    }

    // Sprint
    if (SprintAction) {
      EIC->BindAction(SprintAction, ETriggerEvent::Started, this,
                      &ARailsPlayerCharacter::StartSprint);
      EIC->BindAction(SprintAction, ETriggerEvent::Completed, this,
                      &ARailsPlayerCharacter::StopSprint);
    } else {
      UE_LOG(LogRailsChar, Error, TEXT("SprintAction is null"));
    }

    // Jump
    if (JumpAction) {
      EIC->BindAction(JumpAction, ETriggerEvent::Started, this,
                      &ARailsPlayerCharacter::StartJump);
      EIC->BindAction(JumpAction, ETriggerEvent::Completed, this,
                      &ARailsPlayerCharacter::StopJump);
    } else {
      UE_LOG(LogRailsChar, Error, TEXT("JumpAction is null"));
    }

  } else {
    UE_LOG(LogRailsChar, Error,
           TEXT("Failed to cast to EnhancedInputComponent"));
  }
}

//==================== TICK ====================//

void ARailsPlayerCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (CurrentMode == EPlayerMode::Walking) {
    TraceForInteractable();
  }

  if (CurrentMode == EPlayerMode::Building) {
    UpdateBuildPreview();
  }

  UpdateHeadRotation(DeltaTime);
}

//==================== MOVEMENT INPUT ====================//

void ARailsPlayerCharacter::Move(const FInputActionValue &Value) {
  if (CurrentMode != EPlayerMode::Walking) {
    return;
  }

  const FVector2D Input2D = Value.Get<FVector2D>();
  if (!Controller || Input2D.IsNearlyZero()) {
    return;
  }

  const FRotator ControlRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
  const FVector Forward = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
  const FVector Right = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);

  AddMovementInput(Forward, Input2D.Y);
  AddMovementInput(Right, Input2D.X);
}

void ARailsPlayerCharacter::Look(const FInputActionValue &Value) {
  const FVector2D Input = Value.Get<FVector2D>();
  AddControllerYawInput(Input.X);
  AddControllerPitchInput(Input.Y);
}

//==================== SPRINT ====================//

void ARailsPlayerCharacter::StartSprint() {
  if (CurrentMode != EPlayerMode::Walking) {
    return;
  }

  bIsSprinting = true;
  if (UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    MoveComp->MaxWalkSpeed = SprintSpeed;
  }
  UE_LOG(LogRailsChar, Log, TEXT("Sprint started"));
}

void ARailsPlayerCharacter::StopSprint() {
  bIsSprinting = false;
  if (UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    MoveComp->MaxWalkSpeed = WalkSpeed;
  }
  UE_LOG(LogRailsChar, Log, TEXT("Sprint stopped"));
}

//==================== JUMP ====================//

void ARailsPlayerCharacter::StartJump() {
  if (CurrentMode != EPlayerMode::Walking) {
    return;
  }

  Jump();
  UE_LOG(LogRailsChar, Log, TEXT("Jump started"));
}

void ARailsPlayerCharacter::StopJump() { StopJumping(); }

//==================== INTERACTION ====================//

void ARailsPlayerCharacter::Interact() {
  if (!TargetedInteractable.GetInterface()) {
    UE_LOG(LogRailsChar, Log, TEXT("No interactable target"));
    return;
  }

  if (!IInteractableInterface::Execute_CanInteract(
          TargetedInteractable.GetObject(), GetController())) {
    UE_LOG(LogRailsChar, Warning, TEXT("Cannot interact with this object"));
    return;
  }

  IInteractableInterface::Execute_OnInteract(TargetedInteractable.GetObject(),
                                             GetController());
  UE_LOG(LogRailsChar, Log, TEXT("Interacted with: %s"),
         *TargetedInteractable.GetObject()->GetName());
}

void ARailsPlayerCharacter::TraceForInteractable() {
  if (!Controller || !FirstPersonCamera) {
    return;
  }

  TArray<AActor *> IgnoredActors;
  IgnoredActors.Add(this);
  FHitResult HitResult;

  const bool bHit = FAimTraceService::TraceFromScreenCenter(
      GetWorld(), Cast<APlayerController>(Controller), InteractionDistance,
      InteractionChannel, IgnoredActors, HitResult, false);

  TScriptInterface<IInteractableInterface> NewTarget;

  if (bHit && HitResult.GetActor()) {
    if (HitResult.GetActor()->Implements<UInteractableInterface>()) {
      NewTarget.SetObject(HitResult.GetActor());
      NewTarget.SetInterface(
          Cast<IInteractableInterface>(HitResult.GetActor()));
    }
  }

  if (NewTarget.GetObject() != TargetedInteractable.GetObject()) {
    if (TargetedInteractable.GetInterface()) {
      IInteractableInterface::Execute_OnLookAtEnd(
          TargetedInteractable.GetObject());
    }

    TargetedInteractable = NewTarget;

    if (TargetedInteractable.GetInterface()) {
      IInteractableInterface::Execute_OnLookAtStart(
          TargetedInteractable.GetObject());
    }
  }

#if !UE_BUILD_SHIPPING
  if (bHit) {
    FColor SphereColor = TargetedInteractable.GetInterface()
                             ? FColor::Green
                             : FColor(100, 100, 100, 128);
    DrawDebugSphere(GetWorld(), HitResult.Location, 10.0f, 8, SphereColor,
                    false, 0.1f);
  }
#endif
}

//==================== MODE MANAGEMENT ====================//

void ARailsPlayerCharacter::SetPlayerMode(EPlayerMode NewMode) {
  if (CurrentMode == NewMode) {
    return;
  }

  EPlayerMode OldMode = CurrentMode;
  CurrentMode = NewMode;

  UE_LOG(LogRailsChar, Log, TEXT("Mode changed: %d -> %d"),
         static_cast<int32>(OldMode), static_cast<int32>(NewMode));

  switch (CurrentMode) {
  case EPlayerMode::Walking:
    if (UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
      MoveComp->SetMovementMode(MOVE_Walking);
      MoveComp->MaxWalkSpeed = WalkSpeed;
    }
    break;

  case EPlayerMode::Driving:
    if (UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
      MoveComp->DisableMovement();
    }
    break;

  case EPlayerMode::Building:
    if (UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
      MoveComp->MaxWalkSpeed = 300.f;
    }
    break;
  }
}

void ARailsPlayerCharacter::EnterTrainControlMode(ABaseVehicle *Train) {
  if (!Train) {
    UE_LOG(LogRailsChar, Warning,
           TEXT("Cannot enter train control: Train is null"));
    return;
  }

  CurrentTrain = Train;
  SetPlayerMode(EPlayerMode::Driving);

  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeGameAndUI());
  }

  UE_LOG(LogRailsChar, Log, TEXT("Entered train control mode: %s"),
         *Train->GetName());
}

void ARailsPlayerCharacter::ExitTrainControlMode() {
  if (!CurrentTrain) {
    return;
  }

  CurrentTrain = nullptr;
  SetPlayerMode(EPlayerMode::Walking);

  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    PC->bShowMouseCursor = false;
    PC->SetInputMode(FInputModeGameOnly());
  }

  UE_LOG(LogRailsChar, Log, TEXT("Exited train control mode"));
}

void ARailsPlayerCharacter::ToggleBuildMode() {
  if (CurrentMode == EPlayerMode::Building) {
    ExitCurrentMode();
  } else {
    SetPlayerMode(EPlayerMode::Building);
    UE_LOG(LogRailsChar, Log, TEXT("Entered build mode"));
  }
}

void ARailsPlayerCharacter::ExitCurrentMode() {
  switch (CurrentMode) {
  case EPlayerMode::Driving:
    ExitTrainControlMode();
    break;

  case EPlayerMode::Building:
    CancelBuildPreview();
    SetPlayerMode(EPlayerMode::Walking);
    UE_LOG(LogRailsChar, Log, TEXT("Exited build mode"));
    break;

  default:
    break;
  }
}

//==================== BUILDING SYSTEM ====================//

void ARailsPlayerCharacter::UpdateBuildPreview() {
  // TODO: Implementation
}

void ARailsPlayerCharacter::PlaceBuildObject() {
  // TODO: Implementation
}

void ARailsPlayerCharacter::CancelBuildPreview() {
  if (PreviewObject) {
    PreviewObject->Destroy();
    PreviewObject = nullptr;
  }
}

//==================== ITEM MANAGEMENT ====================//

void ARailsPlayerCharacter::EquipItem(AActor *Item) {
  if (!Item) {
    UE_LOG(LogRailsChar, Warning, TEXT("Cannot equip null item"));
    return;
  }

  if (CurrentHeldItem) {
    UnequipItem();
  }

  CurrentHeldItem = Item;
  Item->AttachToComponent(GetMesh(),
                          FAttachmentTransformRules::SnapToTargetIncludingScale,
                          TEXT("hand_r"));
  HandState = EHandState::HoldingTool;

  UE_LOG(LogRailsChar, Log, TEXT("Equipped item: %s"), *Item->GetName());
}

void ARailsPlayerCharacter::UnequipItem() {
  if (!CurrentHeldItem) {
    return;
  }

  CurrentHeldItem->DetachFromActor(
      FDetachmentTransformRules::KeepWorldTransform);
  CurrentHeldItem = nullptr;
  HandState = EHandState::Empty;

  UE_LOG(LogRailsChar, Log, TEXT("Unequipped item"));
}

//==================== HEAD ROTATION ====================//

void ARailsPlayerCharacter::UpdateHeadRotation(float DeltaTime) {
  // Animation Blueprint logic through Control Rig
}

void ARailsPlayerCharacter::HideHeadForOwner() {
  if (!GetMesh()) {
    return;
  }

  UMaterialInstanceDynamic *HeadMat =
      GetMesh()->CreateDynamicMaterialInstance(0);
  if (HeadMat) {
    // HeadMat->SetScalarParameterValue(TEXT("HideHead"), 1.0f);
  }

  UE_LOG(LogRailsChar, Log,
         TEXT("Head hidden for owner (if material configured)"));
}

//==================== ANIMATION FUNCTIONS ====================//

float ARailsPlayerCharacter::GetMovementSpeed() const {
  if (const UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    return MoveComp->Velocity.Size();
  }
  return 0.0f;
}

float ARailsPlayerCharacter::GetNormalizedSpeed() const {
  if (const UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    const float CurrentSpeed = MoveComp->Velocity.Size();
    const float MaxSpeed = MoveComp->MaxWalkSpeed;
    return MaxSpeed > 0.0f ? FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f)
                           : 0.0f;
  }
  return 0.0f;
}

float ARailsPlayerCharacter::GetMovementDirection() const {
  if (const UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    if (MoveComp->Velocity.SizeSquared() > 0.0f) {
      const FVector VelocityNormal = MoveComp->Velocity.GetSafeNormal2D();
      const FVector ForwardVector = GetActorForwardVector();
      const FVector RightVector = GetActorRightVector();

      const float ForwardDot =
          FVector::DotProduct(VelocityNormal, ForwardVector);
      const float RightDot = FVector::DotProduct(VelocityNormal, RightVector);

      return FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    }
  }
  return 0.0f;
}

bool ARailsPlayerCharacter::IsMoving() const {
  if (const UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    return MoveComp->Velocity.SizeSquared() > 25.0f;
  }
  return false;
}

bool ARailsPlayerCharacter::IsInAir() const {
  if (const UCharacterMovementComponent *MoveComp = GetCharacterMovement()) {
    return MoveComp->IsFalling();
  }
  return false;
}

float ARailsPlayerCharacter::GetTargetSpeed() const {
  return bIsSprinting ? SprintSpeed : WalkSpeed;
}
