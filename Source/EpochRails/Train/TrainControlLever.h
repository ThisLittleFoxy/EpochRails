// TrainControlLever.h
// 3D interactable lever for train control (throttle, brake, reverse)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "TrainControlLever.generated.h"

class ATrainActor;
class UStaticMeshComponent;
class USceneComponent;

/**
 * Type of train control this lever provides
 */
UENUM(BlueprintType)
enum class ETrainLeverType : uint8
{
	Throttle     UMETA(DisplayName = "Throttle"),
	Brake        UMETA(DisplayName = "Brake"),
	Reverse      UMETA(DisplayName = "Reverse Toggle"),
	EmergencyBrake UMETA(DisplayName = "Emergency Brake")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeverValueChanged, ETrainLeverType, LeverType, float, NewValue);

/**
 * 3D interactable lever for train control.
 * Implements visual animation when interacted with.
 */
UCLASS(BlueprintType, Blueprintable)
class EPOCHRAILS_API ATrainControlLever : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ATrainControlLever();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ===== Components =====

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** Lever base (static part) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	/** Lever handle (moving part) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HandleMesh;

	/** Pivot point for lever rotation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> LeverPivot;

	// ===== Configuration =====

	/** Type of control this lever provides */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train")
	ETrainLeverType LeverType = ETrainLeverType::Throttle;

	/** Reference to the train this lever controls */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Train")
	TObjectPtr<ATrainActor> ControlledTrain;

	/** Value change per interaction (for throttle/brake) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ValueStep = 0.25f;

	/** Maximum rotation angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever")
	float MaxRotationAngle = 45.0f;

	/** Rotation axis for lever animation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever")
	FVector RotationAxis = FVector(0.0f, 1.0f, 0.0f);

	/** Speed of lever animation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever")
	float AnimationSpeed = 5.0f;

	/** Maximum interaction distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float MaxInteractionDistance = 150.0f;

	// ===== IInteractableInterface =====

	virtual void OnInteractionFocusBegin_Implementation(ACharacter* PlayerCharacter) override;
	virtual void OnInteractionFocusEnd_Implementation(ACharacter* PlayerCharacter) override;
	virtual bool OnInteract_Implementation(ACharacter* PlayerCharacter) override;
	virtual FText GetInteractionName_Implementation() const override;
	virtual FText GetInteractionAction_Implementation() const override;
	virtual bool CanInteract_Implementation(ACharacter* PlayerCharacter) const override;
	virtual float GetInteractionDistance_Implementation() const override;

	// ===== State =====

	/** Get current lever value (0-1 for throttle/brake) */
	UFUNCTION(BlueprintCallable, Category = "Train")
	float GetCurrentValue() const { return CurrentValue; }

	/** Set lever value directly */
	UFUNCTION(BlueprintCallable, Category = "Train")
	void SetValue(float NewValue);

	/** Reset lever to neutral position */
	UFUNCTION(BlueprintCallable, Category = "Train")
	void ResetToNeutral();

	// ===== Events =====

	/** Broadcast when lever value changes */
	UPROPERTY(BlueprintAssignable, Category = "Train|Events")
	FOnLeverValueChanged OnValueChanged;

	// ===== Blueprint Events =====

	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void BP_OnLeverActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void BP_OnFocusBegin(ACharacter* PlayerCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void BP_OnFocusEnd(ACharacter* PlayerCharacter);

private:
	/** Apply current value to train */
	void ApplyValueToTrain();

	/** Update lever visual rotation */
	void UpdateLeverRotation(float DeltaTime);

	/** Current value (0-1) */
	UPROPERTY(VisibleInstanceOnly, Category = "Debug")
	float CurrentValue = 0.0f;

	/** Target rotation for animation */
	float TargetRotation = 0.0f;

	/** Current rotation for animation */
	float CurrentRotation = 0.0f;

	/** Is reverse engaged (for reverse lever) */
	bool bReverseEngaged = false;

	/** Is emergency brake engaged */
	bool bEmergencyEngaged = false;
};
