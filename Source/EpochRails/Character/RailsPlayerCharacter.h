#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "RailsPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class ABaseVehicle;
class IInteractableInterface;

UENUM(BlueprintType)
enum class EPlayerMode : uint8 {
  Walking UMETA(DisplayName = "Walking"),
  Driving UMETA(DisplayName = "Driving Train"),
  Building UMETA(DisplayName = "Building")
};

/** Состояние рук для анимаций */
UENUM(BlueprintType)
enum class EHandState : uint8 {
  Empty UMETA(DisplayName = "Empty Hands"),
  HoldingTool UMETA(DisplayName = "Holding Tool"),
  HoldingWeapon UMETA(DisplayName = "Holding Weapon")
};

UCLASS()
class EPOCHRAILS_API ARailsPlayerCharacter : public ACharacter {
  GENERATED_BODY()

public:
  ARailsPlayerCharacter();

protected:
  virtual void BeginPlay() override;
  virtual void
  SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;
  virtual void Tick(float DeltaTime) override;

private:
  // ========== Camera ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
            meta = (AllowPrivateAccess = "true"))
  UCameraComponent *FirstPersonCamera;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
            meta = (AllowPrivateAccess = "true"))
  float CameraForwardOffset = 10.0f;

  // ========== Input Actions ==========

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputMappingContext *DefaultMappingContext;

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputAction *MoveAction;

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputAction *LookAction;

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputAction *InteractAction;

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputAction *ToggleBuildModeAction;

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputAction *ExitModeAction;

  UPROPERTY(EditDefaultsOnly, Category = "Input")
  UInputAction *SprintAction;

  // ========== Interaction ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  float InteractionDistance = 500.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_Visibility;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  TScriptInterface<IInteractableInterface> TargetedInteractable;

  // ========== Player Mode ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mode",
            meta = (AllowPrivateAccess = "true"))
  EPlayerMode CurrentMode = EPlayerMode::Walking;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mode",
            meta = (AllowPrivateAccess = "true"))
  ABaseVehicle *CurrentTrain = nullptr;

  // ========== Animation / Hand State ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation",
            meta = (AllowPrivateAccess = "true"))
  EHandState HandState = EHandState::Empty;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation",
            meta = (AllowPrivateAccess = "true"))
  bool bIsSprinting = false;

  // ========== Items ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items",
            meta = (AllowPrivateAccess = "true"))
  AActor *CurrentHeldItem = nullptr;

  // ========== Building System ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building",
            meta = (AllowPrivateAccess = "true"))
  float BuildDistance = 1000.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building",
            meta = (AllowPrivateAccess = "true"))
  TSubclassOf<AActor> PreviewObjectClass;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building",
            meta = (AllowPrivateAccess = "true"))
  AActor *PreviewObject = nullptr;

  // ========== Movement Settings ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  float WalkSpeed = 400.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (AllowPrivateAccess = "true"))
  float SprintSpeed = 600.0f;

public:
  // ========== Movement Input ==========

  void Move(const FInputActionValue &Value);
  void Look(const FInputActionValue &Value);
  void StartSprint();
  void StopSprint();

  // ========== Interaction ==========

  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void Interact();

  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void TraceForInteractable();

  // ========== Mode Management ==========

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void SetPlayerMode(EPlayerMode NewMode);

  UFUNCTION(BlueprintPure, Category = "Mode")
  EPlayerMode GetPlayerMode() const { return CurrentMode; }

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void EnterTrainControlMode(ABaseVehicle *Train);

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void ExitTrainControlMode();

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void ToggleBuildMode();

  UFUNCTION(BlueprintCallable, Category = "Mode")
  void ExitCurrentMode();

  // ========== Building ==========

  UFUNCTION(BlueprintCallable, Category = "Building")
  void UpdateBuildPreview();

  UFUNCTION(BlueprintCallable, Category = "Building")
  void PlaceBuildObject();

  UFUNCTION(BlueprintCallable, Category = "Building")
  void CancelBuildPreview();

  // ========== Items ==========

  UFUNCTION(BlueprintCallable, Category = "Items")
  void EquipItem(AActor *Item);

  UFUNCTION(BlueprintCallable, Category = "Items")
  void UnequipItem();

  // ========== Animation ==========

  /** Обновление поворота головы за камерой */
  void UpdateHeadRotation(float DeltaTime);

  /** Скрыть голову от владельца (опционально) */
  void HideHeadForOwner();

  // ========== Getters ==========

  UFUNCTION(BlueprintPure, Category = "Train")
  ABaseVehicle *GetCurrentTrain() const { return CurrentTrain; }

  UFUNCTION(BlueprintPure, Category = "Camera")
  UCameraComponent *GetFirstPersonCamera() const { return FirstPersonCamera; }

  UFUNCTION(BlueprintPure, Category = "Animation")
  EHandState GetHandState() const { return HandState; }

  UFUNCTION(BlueprintPure, Category = "Animation")
  bool IsSprinting() const { return bIsSprinting; }

  UFUNCTION(BlueprintPure, Category = "Animation")
  FRotator GetControlRotationForAnimBP() const { return GetControlRotation(); }
};
