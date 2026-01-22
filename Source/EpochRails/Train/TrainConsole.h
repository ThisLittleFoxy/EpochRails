// TrainConsole.h
// 3D interactable console for train control

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "TrainConsole.generated.h"

class ATrainActor;
class UStaticMeshComponent;
class UBoxComponent;
class UInputMappingContext;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsoleActivated, ACharacter*, Operator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConsoleDeactivated);

/**
 * Train control console - 3D interactable panel.
 * When player interacts, switches to train control input context.
 * Player stays in walking mode but can control train via input actions.
 */
UCLASS(BlueprintType, Blueprintable)
class EPOCHRAILS_API ATrainConsole : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ATrainConsole();

protected:
	virtual void BeginPlay() override;

public:
	// ===== Components =====

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** Console mesh (panel/dashboard) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ConsoleMesh;

	/** Interaction trigger zone */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionZone;

	// ===== Configuration =====

	/** Reference to the train this console controls */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Train")
	TObjectPtr<ATrainActor> ControlledTrain;

	/** Input mapping context to add when operating console */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> TrainControlMappingContext;

	/** Priority for the train control input context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 InputContextPriority = 1;

	/** Maximum distance for interaction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float MaxInteractionDistance = 200.0f;

	/** Text shown when looking at console */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractionName = NSLOCTEXT("Train", "ConsoleName", "Train Console");

	/** Action text when can interact */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractionActionText = NSLOCTEXT("Train", "ConsoleAction", "Press E to operate");

	/** Action text when already operating */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText StopOperatingText = NSLOCTEXT("Train", "ConsoleStop", "Press E to stop operating");

	// ===== IInteractableInterface =====

	virtual void OnInteractionFocusBegin_Implementation(ACharacter* PlayerCharacter) override;
	virtual void OnInteractionFocusEnd_Implementation(ACharacter* PlayerCharacter) override;
	virtual bool OnInteract_Implementation(ACharacter* PlayerCharacter) override;
	virtual FText GetInteractionName_Implementation() const override;
	virtual FText GetInteractionAction_Implementation() const override;
	virtual bool CanInteract_Implementation(ACharacter* PlayerCharacter) const override;
	virtual float GetInteractionDistance_Implementation() const override;

	// ===== State =====

	/** Check if console is currently being operated */
	UFUNCTION(BlueprintCallable, Category = "Train")
	bool IsBeingOperated() const { return CurrentOperator != nullptr; }

	/** Get the current operator */
	UFUNCTION(BlueprintCallable, Category = "Train")
	ACharacter* GetCurrentOperator() const { return CurrentOperator; }

	/** Force stop operation (e.g., operator moved too far) */
	UFUNCTION(BlueprintCallable, Category = "Train")
	void ForceStopOperation();

	// ===== Events =====

	/** Broadcast when operator starts using console */
	UPROPERTY(BlueprintAssignable, Category = "Train|Events")
	FOnConsoleActivated OnConsoleActivated;

	/** Broadcast when operator stops using console */
	UPROPERTY(BlueprintAssignable, Category = "Train|Events")
	FOnConsoleDeactivated OnConsoleDeactivated;

	// ===== Blueprint Events =====

	/** Called when operator starts using console */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void BP_OnOperationStarted(ACharacter* Operator);

	/** Called when operator stops using console */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void BP_OnOperationStopped();

	/** Called when focus begins (for visual feedback) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void BP_OnFocusBegin(ACharacter* PlayerCharacter);

	/** Called when focus ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Train")
	void BP_OnFocusEnd(ACharacter* PlayerCharacter);

private:
	/** Start operating the console */
	void StartOperation(ACharacter* Operator);

	/** Stop operating the console */
	void StopOperation();

	/** Add train control input context to player */
	void AddInputContext(APlayerController* PC);

	/** Remove train control input context from player */
	void RemoveInputContext(APlayerController* PC);

	/** Current operator character */
	UPROPERTY()
	TObjectPtr<ACharacter> CurrentOperator;

	/** Cached player controller for input context */
	UPROPERTY()
	TObjectPtr<APlayerController> CachedController;
};
