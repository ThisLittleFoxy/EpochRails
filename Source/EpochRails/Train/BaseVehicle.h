#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class ULocomotionComponent;
class UResourceInventory;
class USplineComponent;
class ARailSplineActor;

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

private:
  void InitializeRailSpline();
};
