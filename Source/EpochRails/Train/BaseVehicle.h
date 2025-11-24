#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class ULocomotionComponent;
class UResourceInventory;
class USplineComponent;
class ARailSplineActor;

// Делегат для события смерти
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleDestroyed, ABaseVehicle *,
                                            DestroyedVehicle);

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
  // ----- Health -----
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
  float MaxHealth = 500.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
  float CurrentHealth = 0.0f;

  // ----- Movement -----
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  float MaxSpeed = 1000.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
  float CurrentSpeed = 0.0f;

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Явное назначение рельсов
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (DisplayName = "Assigned Rail Spline"))
  ARailSplineActor *AssignedRailSpline = nullptr;

  // Fallback: автопоиск, если не назначено вручную
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (DisplayName = "Auto-Find Nearest Rail"))
  bool bAutoFindNearestRail = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
            meta = (DisplayName = "Rail Search Radius",
                    EditCondition = "bAutoFindNearestRail"))
  float RailSearchRadius = 1000.0f;

  // ----- Systems -----
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
  UResourceInventory *ResourceInventory;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
  ULocomotionComponent *LocomotionComponent;

public:
  // Damage handling
  virtual float TakeDamage(float DamageAmount,
                           struct FDamageEvent const &DamageEvent,
                           class AController *EventInstigator,
                           AActor *DamageCauser) override;

  UFUNCTION(BlueprintCallable, Category = "Movement")
  void SetThrottle(float Value);

  UFUNCTION(BlueprintPure, Category = "Health")
  float GetHealthPercent() const;

  UFUNCTION(BlueprintPure, Category = "Systems")
  UResourceInventory *GetResourceInventory() const { return ResourceInventory; }

  UFUNCTION(BlueprintPure, Category = "Systems")
  ULocomotionComponent *GetLocomotionComponent() const {
    return LocomotionComponent;
  }

  // Manually assign rail spline
  UFUNCTION(BlueprintCallable, Category = "Movement")
  void SetRailSpline(ARailSplineActor *NewRailSpline);

  // Check if vehicle is destroyed
  UFUNCTION(BlueprintPure, Category = "Health")
  bool IsDestroyed() const { return CurrentHealth <= 0.0f; }

public:
  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Событие смерти
  UPROPERTY(BlueprintAssignable, Category = "Health")
  FOnVehicleDestroyed OnVehicleDestroyed;

protected:
  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Обработка смерти
  UFUNCTION(BlueprintNativeEvent, Category = "Health")
  void HandleDestruction();
  virtual void HandleDestruction_Implementation();

private:
  void InitializeRailSpline();
  ARailSplineActor *FindNearestRail();

  // Флаг для предотвращения множественных вызовов смерти
  bool bIsDestroyed = false;
};
