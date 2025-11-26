#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class ULocomotionComponent;
class UResourceInventory;
class USplineComponent;
class ARailSplineActor;
class UStaticMeshComponent;
class USpotLightComponent;

// ========== НОВЫЕ ТИПЫ ==========

// Enum для типов транспорта
UENUM(BlueprintType)
enum class EVehicleType : uint8 {
  Locomotive UMETA(DisplayName = "Locomotive"),
  CargoWagon UMETA(DisplayName = "Cargo Wagon"),
  PassengerWagon UMETA(DisplayName = "Passenger Wagon"),
  CustomWagon UMETA(DisplayName = "Custom Wagon")
};

// Структура для ячейки сетки строительства
USTRUCT(BlueprintType)
struct FBuildGridCell {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  int32 X = 0;

  UPROPERTY(BlueprintReadWrite)
  int32 Y = 0;

  UPROPERTY(BlueprintReadWrite)
  bool bOccupied = false;

  UPROPERTY(BlueprintReadWrite)
  AActor *PlacedObject = nullptr;

  FBuildGridCell() {}
  FBuildGridCell(int32 InX, int32 InY)
      : X(InX), Y(InY), bOccupied(false), PlacedObject(nullptr) {}
};

// Делегаты
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleDestroyed, ABaseVehicle *,
                                            DestroyedVehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThrottleChanged, float,
                                             NewThrottle, float, MaxThrottle);

UCLASS()
class EPOCHRAILS_API ABaseVehicle : public APawn {
  GENERATED_BODY()

public:
  ABaseVehicle();

  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

protected:
  // ========== Визуальные компоненты ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
  UStaticMeshComponent *VehicleMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
  UStaticMeshComponent *CabinMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
  USceneComponent *FrontCouplerPoint;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
  USceneComponent *RearCouplerPoint;

  // ========== Health ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
  float MaxHealth = 500.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
  float CurrentHealth = 0.0f;

  // ========== Movement ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  float MaxSpeed = 1000.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
  float CurrentSpeed = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (DisplayName = "Assigned Rail Spline"))
  ARailSplineActor *AssignedRailSpline = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (DisplayName = "Auto-Find Nearest Rail"))
  bool bAutoFindNearestRail = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (DisplayName = "Rail Search Radius",
                    EditCondition = "bAutoFindNearestRail"))
  float RailSearchRadius = 1000.0f;

  // ========== Vehicle Type ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Type")
  EVehicleType VehicleType = EVehicleType::Locomotive;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Type")
  bool bIsLocomotive = true;

  // ========== Train Composition ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train Composition")
  ABaseVehicle *LeadingVehicle = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train Composition")
  TArray<ABaseVehicle *> AttachedWagons;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Composition")
  int32 MaxAttachedWagons = 5;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Composition")
  float WagonLength = 600.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Composition")
  float CouplerGap = 50.0f;

  // ========== Engine ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine")
  bool bEngineRunning = true;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine")
  float TargetThrottle = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
  float ThrottleChangeRate = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
  float EngineEfficiency = 1.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
  bool bRequiresFuel = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine")
  float CurrentFuel = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
  float MaxFuel = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
  float FuelConsumptionRate = 1.0f;

  // ========== Building System ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
  bool bAllowBuilding = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building",
            meta = (EditCondition = "bAllowBuilding"))
  int32 GridSizeX = 5;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building",
            meta = (EditCondition = "bAllowBuilding"))
  int32 GridSizeY = 3;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building",
            meta = (EditCondition = "bAllowBuilding"))
  float CellSize = 100.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
  TArray<FBuildGridCell> BuildGrid;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
  TArray<AActor *> PlacedObjects;

  // ========== Lights ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
  bool bHasHeadlights = true;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lights")
  bool bHeadlightsEnabled = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lights")
  USpotLightComponent *HeadlightLeft;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lights")
  USpotLightComponent *HeadlightRight;

  // ========== Systems ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
  UResourceInventory *ResourceInventory;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
  ULocomotionComponent *LocomotionComponent;

public:
  // ========== Health ==========

  virtual float TakeDamage(float DamageAmount,
                           struct FDamageEvent const &DamageEvent,
                           class AController *EventInstigator,
                           AActor *DamageCauser) override;

  UFUNCTION(BlueprintPure, Category = "Health")
  float GetHealthPercent() const;

  UFUNCTION(BlueprintPure, Category = "Health")
  bool IsDestroyed() const { return CurrentHealth <= 0.0f; }

  // ========== Movement Control ==========

  UFUNCTION(BlueprintCallable, Category = "Movement")
  void SetThrottle(float Value);

  UFUNCTION(BlueprintCallable, Category = "Movement")
  void SetRailSpline(ARailSplineActor *NewRailSpline);

  // ========== Engine Control ==========

  UFUNCTION(BlueprintCallable, Category = "Engine")
  void StartEngine();

  UFUNCTION(BlueprintCallable, Category = "Engine")
  void StopEngine();

  UFUNCTION(BlueprintCallable, Category = "Engine")
  void ToggleEngine();

  UFUNCTION(BlueprintPure, Category = "Engine")
  bool IsEngineRunning() const { return bEngineRunning; }

  UFUNCTION(BlueprintCallable, Category = "Engine")
  void SetTargetThrottle(float Value);

  UFUNCTION(BlueprintCallable, Category = "Engine")
  void IncreaseThrottle(float Amount = 0.1f);

  UFUNCTION(BlueprintCallable, Category = "Engine")
  void DecreaseThrottle(float Amount = 0.1f);

  UFUNCTION(BlueprintCallable, Category = "Engine")
  void EmergencyBrake();

  UFUNCTION(BlueprintPure, Category = "Engine")
  float GetFuelPercent() const {
    return MaxFuel > 0.0f ? CurrentFuel / MaxFuel : 0.0f;
  }

  // ========== Train Composition ==========

  UFUNCTION(BlueprintCallable, Category = "Train Composition")
  bool AttachWagon(ABaseVehicle *Wagon);

  UFUNCTION(BlueprintCallable, Category = "Train Composition")
  bool DetachWagon(ABaseVehicle *Wagon);

  UFUNCTION(BlueprintCallable, Category = "Train Composition")
  void DetachAllWagons();

  UFUNCTION(BlueprintPure, Category = "Train Composition")
  int32 GetWagonCount() const { return AttachedWagons.Num(); }

  UFUNCTION(BlueprintPure, Category = "Train Composition")
  float GetTotalTrainMass() const;

  UFUNCTION(BlueprintCallable, Category = "Train Composition")
  void SetLeadingVehicle(ABaseVehicle *Locomotive);

  UFUNCTION(BlueprintPure, Category = "Train Composition")
  ABaseVehicle *GetLeadingVehicle() const { return LeadingVehicle; }

  // ========== Lights ==========

  UFUNCTION(BlueprintCallable, Category = "Lights")
  void ToggleHeadlights();

  UFUNCTION(BlueprintCallable, Category = "Lights")
  void SetHeadlights(bool bEnabled);

  // ========== Building System ==========

  UFUNCTION(BlueprintCallable, Category = "Building")
  void InitializeBuildGrid();

  UFUNCTION(BlueprintPure, Category = "Building")
  FVector GetGridCellWorldLocation(int32 X, int32 Y) const;

  UFUNCTION(BlueprintPure, Category = "Building")
  bool IsGridCellValid(int32 X, int32 Y) const;

  UFUNCTION(BlueprintPure, Category = "Building")
  bool IsGridCellOccupied(int32 X, int32 Y) const;

  UFUNCTION(BlueprintCallable, Category = "Building")
  bool PlaceObjectAtCell(int32 X, int32 Y, TSubclassOf<AActor> ObjectClass);

  UFUNCTION(BlueprintCallable, Category = "Building")
  bool RemoveObjectAtCell(int32 X, int32 Y);

  UFUNCTION(BlueprintCallable, Category = "Building")
  void ClearAllObjects();

  // ========== Getters ==========

  UFUNCTION(BlueprintPure, Category = "Systems")
  UResourceInventory *GetResourceInventory() const { return ResourceInventory; }

  UFUNCTION(BlueprintPure, Category = "Systems")
  ULocomotionComponent *GetLocomotionComponent() const {
    return LocomotionComponent;
  }

  // ========== Events ==========

  UPROPERTY(BlueprintAssignable, Category = "Health")
  FOnVehicleDestroyed OnVehicleDestroyed;

  UPROPERTY(BlueprintAssignable, Category = "Engine")
  FOnThrottleChanged OnThrottleChanged;

protected:
  UFUNCTION(BlueprintNativeEvent, Category = "Health")
  void HandleDestruction();
  virtual void HandleDestruction_Implementation();

private:
  void InitializeRailSpline();
  ARailSplineActor *FindNearestRail();
  void CreateVisualComponents();
  void SetupMeshForVehicleType();
  void UpdateThrottle(float DeltaTime);
  void UpdateFuelConsumption(float DeltaTime);
  void UpdateWagonPositions();
  int32 GetGridIndex(int32 X, int32 Y) const;

  bool bIsDestroyed = false;
};
