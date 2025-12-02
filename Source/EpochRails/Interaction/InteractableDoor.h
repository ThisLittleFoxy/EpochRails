// InteractableDoor.h
// Door interactable - allows opening/closing doors

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableComponent.h"
#include "InteractionTypes.h"
#include "InteractableDoor.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;

/**
 * Delegate for door events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, bool, bIsOpen);

/**
 * Interactable component for doors
 * Supports both hinge (rotation) and slide (translation) animations
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class EPOCHRAILS_API UInteractableDoor : public UInteractableComponent {
  GENERATED_BODY()

public:
  UInteractableDoor();

protected:
  // ========== DOOR SETTINGS ==========

  /** Type of door animation */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
  EDoorAnimationType AnimationType = EDoorAnimationType::Hinge;

  /** Duration of open/close animation in seconds */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door",
            meta = (ClampMin = "0.1", ClampMax = "5.0"))
  float AnimationDuration = 1.0f;

  /** Curve for animation (optional, linear if not set) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
  UCurveFloat *AnimationCurve = nullptr;

  /** Auto-close door after delay (0 = never auto-close) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door",
            meta = (ClampMin = "0.0", ClampMax = "60.0"))
  float AutoCloseDelay = 0.0f;

  /** Lock door during train movement */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
  bool bLockDuringTrainMovement = true;

  /** Play sound when opening */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
  USoundBase *OpenSound = nullptr;

  /** Play sound when closing */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
  USoundBase *CloseSound = nullptr;

  // ========== HINGE DOOR SETTINGS ==========

  /** Rotation angle for hinge doors (degrees) */
  UPROPERTY(
      EditAnywhere, BlueprintReadWrite, Category = "Door|Hinge",
      meta = (EditCondition = "AnimationType == EDoorAnimationType::Hinge",
              ClampMin = "-180.0", ClampMax = "180.0"))
  float HingeRotationAngle = 90.0f;

  /** Rotation axis for hinge doors */
  UPROPERTY(
      EditAnywhere, BlueprintReadWrite, Category = "Door|Hinge",
      meta = (EditCondition = "AnimationType == EDoorAnimationType::Hinge"))
  FVector HingeRotationAxis = FVector(0.0f, 0.0f, 1.0f);

  // ========== SLIDE DOOR SETTINGS ==========

  /** Slide offset for sliding doors */
  UPROPERTY(
      EditAnywhere, BlueprintReadWrite, Category = "Door|Slide",
      meta = (EditCondition = "AnimationType == EDoorAnimationType::Slide"))
  FVector SlideOffset = FVector(0.0f, 150.0f, 0.0f);

  // ========== STATE ==========

  /** Is door currently open */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
  bool bIsOpen = false;

  /** Is door currently animating */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
  bool bIsAnimating = false;

  /** Current animation progress (0.0 = closed, 1.0 = open) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
  float AnimationProgress = 0.0f;

  /** Cached door mesh component */
  UPROPERTY()
  USceneComponent *DoorComponent = nullptr;

  /** Initial transform of door (for animation reference) */
  FTransform InitialTransform;

  /** Timer handle for auto-close */
  FTimerHandle AutoCloseTimer;

  /** Current animation time */
  float CurrentAnimationTime = 0.0f;

  /** Is animation going forward (opening) */
  bool bAnimatingForward = true;

  // ========== LIFECYCLE ==========

  virtual void BeginPlay() override;
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  // ========== DOOR LOGIC ==========

  /**
   * Open the door
   */
  UFUNCTION(BlueprintCallable, Category = "Door")
  virtual void Open();

  /**
   * Close the door
   */
  UFUNCTION(BlueprintCallable, Category = "Door")
  virtual void Close();

  /**
   * Toggle door state
   */
  UFUNCTION(BlueprintCallable, Category = "Door")
  virtual void Toggle();

  /**
   * Find door component (mesh to animate)
   */
  UFUNCTION(BlueprintCallable, Category = "Door")
  void FindDoorComponent();

  /**
   * Start animation
   */
  void StartAnimation(bool bOpenDirection);

  /**
   * Update door transform based on progress
   */
  void UpdateDoorTransform(float Progress);

  /**
   * Calculate transform for hinge door
   */
  FTransform CalculateHingeTransform(float Progress) const;

  /**
   * Calculate transform for slide door
   */
  FTransform CalculateSlideTransform(float Progress) const;

  /**
   * Start auto-close timer
   */
  void StartAutoCloseTimer();

  /**
   * Clear auto-close timer
   */
  void ClearAutoCloseTimer();

  /**
   * Auto-close callback
   */
  UFUNCTION()
  void OnAutoClose();

public:
  // ========== EVENTS ==========

  /** Called when door state changes */
  UPROPERTY(BlueprintAssignable, Category = "Door|Events")
  FOnDoorStateChanged OnDoorStateChanged;

  /** Called when door finishes opening */
  UPROPERTY(BlueprintAssignable, Category = "Door|Events")
  FOnDoorStateChanged OnDoorOpened;

  /** Called when door finishes closing */
  UPROPERTY(BlueprintAssignable, Category = "Door|Events")
  FOnDoorStateChanged OnDoorClosed;

  // ========== OVERRIDES ==========

  virtual void Interact(ACharacter *Character) override;
  virtual bool CanInteract(ACharacter *Character) const override;

  // ========== QUERY API ==========

  /** Is door open */
  UFUNCTION(BlueprintPure, Category = "Door")
  bool IsOpen() const { return bIsOpen; }

  /** Is door closed */
  UFUNCTION(BlueprintPure, Category = "Door")
  bool IsClosed() const { return !bIsOpen; }

  /** Is door animating */
  UFUNCTION(BlueprintPure, Category = "Door")
  bool IsAnimating() const { return bIsAnimating; }

  /** Get animation progress (0.0 to 1.0) */
  UFUNCTION(BlueprintPure, Category = "Door")
  float GetAnimationProgress() const { return AnimationProgress; }

  /** Set door state immediately (no animation) */
  UFUNCTION(BlueprintCallable, Category = "Door")
  void SetDoorStateImmediate(bool bOpen);
};
