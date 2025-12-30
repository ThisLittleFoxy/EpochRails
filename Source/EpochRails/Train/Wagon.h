// Wagon.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wagon.generated.h"

/**
 * Base wagon class - empty wheeled platform for building
 * Wagons follow train along the spline with fixed distance
 */
UCLASS(Blueprintable)
class EPOCHRAILS_API AWagon : public AActor {
  GENERATED_BODY()

public:
  AWagon();

  /** Front attachment point - visual reference */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *FrontAttachmentPoint;

  /** Rear attachment point - visual reference */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *RearAttachmentPoint;

protected:
  // ========== Components ==========

  /** Root component for the wagon */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *WagonRoot;

  /** Wagon platform mesh (empty floor) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *PlatformMesh;

  /** Wheel meshes for visual feedback */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TArray<UStaticMeshComponent *> WheelMeshes;

  /** Building zone - area where player can place structures */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  class UBoxComponent *BuildingZone;

  // ========== Wagon Properties ==========

/** Used only as fallback if attachment points are missing */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Follow")
  float FollowDistance = 800.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon")
  float WagonWeight = 1000.0f;

  /** Gap between couplers (cm) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Follow")
  float CouplingGap = 50.0f;

  /** Extra vertical offset from rails (cm). Applied along spline Up vector. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Follow")
  float HeightOffset = 0.0f;

  /** How fast wagon interpolates to target distance */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Follow")
  float FollowInterpSpeed = 5.0f;

  /** Current distance along spline */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wagon")
  float CurrentSplineDistance = 0.0f;

  /** Pointer to next wagon in chain */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wagon")
  AWagon *NextWagon = nullptr;

  /** Pointer to previous vehicle */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wagon")
  AActor *PreviousVehicle = nullptr;

private:
  /** Cached spline component from train */
  class USplineComponent *CachedSplineComponent = nullptr;

  /** Target distance to follow */
  float TargetDistance = 0.0f;

public:
  // ========== Lifecycle ==========

  virtual void Tick(float DeltaTime) override;

  // ========== Public API ==========

  /** Initialize wagon with spline and leader */
  /** Initialize wagon with spline and leader */
  UFUNCTION(BlueprintCallable, Category = "Wagon")
  void Initialize(class USplineComponent *Spline, AActor *Leader);

  /** Set target distance for smooth following */
  UFUNCTION(BlueprintCallable, Category = "Wagon")
  void SetTargetDistance(float Distance);

  /** Link next wagon */
  UFUNCTION(BlueprintCallable, Category = "Wagon")
  void SetNextWagon(AWagon *Wagon);

  /** Detach from chain */
  UFUNCTION(BlueprintCallable, Category = "Wagon")
  void DetachFromChain();

  /** Get total weight */
  UFUNCTION(BlueprintPure, Category = "Wagon")
  float GetTotalWeight() const;

  /** Get current distance */
  UFUNCTION(BlueprintPure, Category = "Wagon")
  float GetCurrentDistance() const { return CurrentSplineDistance; }
};
