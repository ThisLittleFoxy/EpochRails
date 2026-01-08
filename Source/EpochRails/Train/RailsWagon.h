// RailsWagon.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RailsWagon.generated.h"

class UFloatingPawnMovement;
class USplineComponent;
class UBoxComponent;
class ARailsTrain;

/**
 * Base wagon class - a platform that follows the train along the spline.
 * Players can place any structures on the platform and they will move with the wagon.
 */
UCLASS(Blueprintable)
class EPOCHRAILS_API ARailsWagon : public APawn {
  GENERATED_BODY()

public:
  ARailsWagon();

  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ===== Chain API =====

  /** Attach this wagon to a leader (train or another wagon) */
  UFUNCTION(BlueprintCallable, Category = "Wagon|Chain")
  void AttachToLeader(AActor *Leader, USplineComponent *Spline);

  /** Detach from the chain */
  UFUNCTION(BlueprintCallable, Category = "Wagon|Chain")
  void Detach();

  /** Get the rear coupler for attaching next wagon */
  UFUNCTION(BlueprintPure, Category = "Wagon|Chain")
  USceneComponent *GetRearCoupler() const { return RearCoupler; }

  /** Get current distance along the spline */
  UFUNCTION(BlueprintPure, Category = "Wagon|Chain")
  float GetCurrentSplineDistance() const { return CurrentSplineDistance; }

  /** Set the next wagon in chain */
  UFUNCTION(BlueprintCallable, Category = "Wagon|Chain")
  void SetNextWagon(ARailsWagon *Wagon) { NextWagon = Wagon; }

  /** Get the next wagon in chain */
  UFUNCTION(BlueprintPure, Category = "Wagon|Chain")
  ARailsWagon *GetNextWagon() const { return NextWagon.Get(); }

  // ===== Structure Placement API =====

  /** Check if a structure can be placed at the given world location */
  UFUNCTION(BlueprintCallable, Category = "Wagon|Building")
  bool CanPlaceStructure(const FVector &WorldLocation, const FVector &StructureExtent) const;

  /** Place a structure on the wagon platform (attaches it to wagon) */
  UFUNCTION(BlueprintCallable, Category = "Wagon|Building")
  bool PlaceStructure(AActor *Structure);

  /** Remove a structure from the wagon platform */
  UFUNCTION(BlueprintCallable, Category = "Wagon|Building")
  bool RemoveStructure(AActor *Structure);

  /** Get all structures placed on this wagon */
  UFUNCTION(BlueprintPure, Category = "Wagon|Building")
  TArray<AActor *> GetPlacedStructures() const;

  /** Get the buildable zone bounds in local space */
  UFUNCTION(BlueprintPure, Category = "Wagon|Building")
  FBox GetBuildableZoneBounds() const;

protected:
  // ===== Components =====

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USceneComponent> Root = nullptr;

  /** The platform mesh - floor of the wagon */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UStaticMeshComponent> PlatformMesh = nullptr;

  /** Movement component for smooth interpolated movement */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UFloatingPawnMovement> Movement = nullptr;

  /** Trigger for detecting players on the platform */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UBoxComponent> PlatformTrigger = nullptr;

  /** Visual bounds for the buildable area (editor only) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UBoxComponent> BuildableZone = nullptr;

  /** Front coupler attachment point */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USceneComponent> FrontCoupler = nullptr;

  /** Rear coupler attachment point for next wagon */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USceneComponent> RearCoupler = nullptr;

  // ===== Platform Settings =====

  /** Size of the platform (X = length, Y = width, Z = height) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Platform")
  FVector PlatformSize = FVector(400.0f, 200.0f, 20.0f);

  /** Maximum height for structures placed on the platform */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Platform")
  float MaxBuildHeight = 300.0f;

  // ===== Movement Settings =====

  /** Distance to maintain from the leader vehicle */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Movement")
  float FollowDistance = 500.0f;

  /** Speed of position/rotation interpolation */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wagon|Movement")
  float InterpSpeed = 8.0f;

  // ===== Chain State =====

  /** The vehicle this wagon follows (train or previous wagon) */
  UPROPERTY(BlueprintReadOnly, Category = "Wagon|Chain")
  TWeakObjectPtr<AActor> LeaderVehicle;

  /** Next wagon in the chain */
  UPROPERTY(BlueprintReadOnly, Category = "Wagon|Chain")
  TWeakObjectPtr<ARailsWagon> NextWagon;

  /** Cached spline component for path following */
  UPROPERTY()
  TObjectPtr<USplineComponent> CachedSpline = nullptr;

  /** Current distance along the spline */
  float CurrentSplineDistance = 0.0f;

  // ===== Structures =====

  /** All structures placed on this wagon */
  UPROPERTY(BlueprintReadOnly, Category = "Wagon|Building")
  TArray<TWeakObjectPtr<AActor>> PlacedStructures;

  // ===== Internal Methods =====

  /** Get the leader's current spline distance */
  float GetLeaderSplineDistance() const;

  /** Update position and rotation based on spline */
  void UpdateMovement(float DeltaTime);
};
