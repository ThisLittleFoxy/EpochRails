// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "RailsPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class ABaseVehicle;
class IInteractableInterface;

// Hand state enum
UENUM(BlueprintType)
enum class EHandState : uint8 {
  Empty UMETA(DisplayName = "Empty"),
  HoldingTool UMETA(DisplayName = "Holding Tool"),
  HoldingWeapon UMETA(DisplayName = "Holding Weapon"),
  HoldingItem UMETA(DisplayName = "Holding Item")
};

// Player mode enum
UENUM(BlueprintType)
enum class EPlayerMode : uint8 {
  Walking UMETA(DisplayName = "Walking"),
  Driving UMETA(DisplayName = "Driving"),
  Building UMETA(DisplayName = "Building")
};

UCLASS()
class EPOCHRAILS_API ARailsPlayerCharacter : public ACharacter {
  GENERATED_BODY()

public:
  ARailsPlayerCharacter();

protected:
  virtual void BeginPlay() override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

public:
  virtual void Tick(float DeltaTime) override;

  //==================== COMPONENTS ====================//

  // Spring arm for camera smoothing and stability
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
            meta = (AllowPrivateAccess = "true"))
  USpringArmComponent *CameraArm;

  // First person camera - created in C++ but configurable in Blueprint
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
            meta = (AllowPrivateAccess = "true"))
  UCameraComponent *FirstPersonCamera;

  //==================== INPUT ====================//

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputMappingContext *DefaultMappingContext;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputAction *MoveAction;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputAction *LookAction;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputAction *InteractAction;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputAction *ToggleBuildModeAction;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputAction *ExitModeAction;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputAction *SprintAction;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  UInputAction *JumpAction;

  //==================== MOVEMENT ====================//

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
  float WalkSpeed = 200.0f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
  float SprintSpeed = 400.0f;

  UPROPERTY(BlueprintReadOnly, Category = "Movement")
  bool bIsSprinting;

  //==================== INTERACTION ====================//

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
  float InteractionDistance = 500.0f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
  TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_Visibility;

  UPROPERTY(BlueprintReadOnly, Category = "Interaction")
  TScriptInterface<IInteractableInterface> TargetedInteractable;

  //==================== CAMERA ====================//

  // Socket name to attach camera to (for animation support outside train)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
  FName CameraSocketName = TEXT("head");

  // Camera offset from socket (used when camera is attached back to mesh)
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
  FVector CameraRelativeLocation = FVector(15.0f, 0.0f, 0.0f);

  // Camera rotation offset from socket
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
  FRotator CameraRelativeRotation = FRotator::ZeroRotator;

  // Legacy - keep for backward compatibility
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
  float CameraForwardOffset = 15.0f;

  //==================== HAND STATE ====================//

  UPROPERTY(BlueprintReadOnly, Category = "Character")
  EHandState HandState;

  UPROPERTY(BlueprintReadOnly, Category = "Character")
  AActor *CurrentHeldItem;

  //==================== PLAYER MODE ====================//

  UPROPERTY(BlueprintReadOnly, Category = "Character")
  EPlayerMode CurrentMode;

  UPROPERTY(BlueprintReadOnly, Category = "Character")
  ABaseVehicle *CurrentTrain;

  //==================== TRAIN MOVEMENT SYNC ====================//

  // Reference to train interior player is currently inside (for walking mode)
  UPROPERTY(BlueprintReadWrite, Category = "Train")
  ABaseVehicle *CurrentTrainInterior;

  //==================== BUILDING ====================//

  UPROPERTY(BlueprintReadOnly, Category = "Building")
  AActor *PreviewObject;

  //==================== INPUT FUNCTIONS ====================//

protected:
  void Move(const FInputActionValue &Value);
  void Look(const FInputActionValue &Value);
  void Interact();
  void StartSprint();
  void StopSprint();
  void StartJump();
  void StopJump();

  //==================== INTERACTION FUNCTIONS ====================//

  void TraceForInteractable();

  //==================== MODE MANAGEMENT ====================//

public:
  UFUNCTION(BlueprintCallable, Category = "Character")
  void SetPlayerMode(EPlayerMode NewMode);

  UFUNCTION(BlueprintCallable, Category = "Character")
  void EnterTrainControlMode(ABaseVehicle *Train);

  UFUNCTION(BlueprintCallable, Category = "Character")
  void ExitTrainControlMode();

  void ToggleBuildMode();
  void ExitCurrentMode();

  //==================== BUILDING FUNCTIONS ====================//

  void UpdateBuildPreview();
  void PlaceBuildObject();
  void CancelBuildPreview();

  //==================== ITEM MANAGEMENT ====================//

  UFUNCTION(BlueprintCallable, Category = "Character")
  void EquipItem(AActor *Item);

  UFUNCTION(BlueprintCallable, Category = "Character")
  void UnequipItem();

  //==================== HEAD ROTATION ====================//

  void UpdateHeadRotation(float DeltaTime);
  void HideHeadForOwner();

  //==================== ANIMATION ====================//

  // Get current movement speed
  UFUNCTION(BlueprintPure, Category = "Animation")
  float GetMovementSpeed() const;

  // Get normalized speed (0-1 range for walk-to-sprint)
  UFUNCTION(BlueprintPure, Category = "Animation")
  float GetNormalizedSpeed() const;

  // Get movement direction relative to character
  UFUNCTION(BlueprintPure, Category = "Animation")
  float GetMovementDirection() const;

  // Check if character is moving
  UFUNCTION(BlueprintPure, Category = "Animation")
  bool IsMoving() const;

  // Check if character is in air
  UFUNCTION(BlueprintPure, Category = "Animation")
  bool IsInAir() const;

  // Get current target speed (walk or sprint)
  UFUNCTION(BlueprintPure, Category = "Animation")
  float GetTargetSpeed() const;

  //==================== PRIVATE ====================//

private:
  // Manual train synchronization variables (used together with SetBase)
  FVector LastTrainLocation;
  bool bTrainSyncInitialized;
};
