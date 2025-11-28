// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

class ULocomotionComponent;
class ARailSplineActor;
class UStaticMeshComponent;
class USceneComponent;
class UCameraComponent;
class USpringArmComponent;

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

  // Root component
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USceneComponent> RootComp;

  // Main vehicle mesh
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UStaticMeshComponent> VehicleMesh;

  // Camera components
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<USpringArmComponent> SpringArm;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UCameraComponent> Camera;

  // Locomotion component for movement
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
  // Cached reference to current driver
  UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
  TObjectPtr<APawn> CurrentDriver;

  // Apply mesh transform settings
  void ApplyVehicleMeshTransform();

  // Input handling
  void MoveForward(float Value);
  void MoveBackward(float Value);
  void Brake(float Value);
};
