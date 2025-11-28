// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "BaseVehicle.generated.h"

class ULocomotionComponent;
class ARailSplineActor;
class UStaticMeshComponent;
class USceneComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class EPOCHRAILS_API ABaseVehicle : public APawn {
  GENERATED_BODY()

public:
  ABaseVehicle();

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

  // Components
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USceneComponent> RootComp;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UStaticMeshComponent> VehicleMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USpringArmComponent> SpringArm;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UCameraComponent> Camera;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<ULocomotionComponent> LocomotionComp;

  // Vehicle mesh transform settings
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
  FVector VehicleMeshScale;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
  FRotator VehicleMeshRotation;

  // Rail spline reference
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
  TObjectPtr<ARailSplineActor> CurrentRailSpline;

  // Enhanced Input System
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputMappingContext> VehicleMappingContext;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> ThrottleAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> BrakeAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> ExitAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
  int32 InputMappingPriority;

  // Player interaction
  UFUNCTION(BlueprintCallable, Category = "Vehicle")
  void EnterVehicle(APawn *Player);

  UFUNCTION(BlueprintCallable, Category = "Vehicle")
  void ExitVehicle();

  UFUNCTION(BlueprintPure, Category = "Vehicle")
  bool IsOccupied() const { return CurrentDriver != nullptr; }

  UFUNCTION(BlueprintPure, Category = "Vehicle")
  APawn *GetCurrentDriver() const { return CurrentDriver; }

protected:
  UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
  TObjectPtr<APawn> CurrentDriver;

  void ApplyVehicleMeshTransform();

  // Enhanced Input callbacks
  void OnThrottle(const FInputActionValue &Value);
  void OnBrake(const FInputActionValue &Value);
  void OnExitVehicle(const FInputActionValue &Value);

  // Input context management
  void AddInputMappingContext();
  void RemoveInputMappingContext();
};
