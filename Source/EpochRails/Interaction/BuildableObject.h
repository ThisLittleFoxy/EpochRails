#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableObject.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EBuildableType : uint8 {
  Storage UMETA(DisplayName = "Storage Container"),
  Turret UMETA(DisplayName = "Defense Turret"),
  Generator UMETA(DisplayName = "Generator"),
  Workbench UMETA(DisplayName = "Workbench"),
  Decoration UMETA(DisplayName = "Decoration"),
  Furniture UMETA(DisplayName = "Furniture")
};

/**
 * Базовый класс для всех объектов, которые можно строить на вагонах
 */
UCLASS()
class EPOCHRAILS_API ABuildableObject : public AActor {
  GENERATED_BODY()

public:
  ABuildableObject();

  virtual void BeginPlay() override;

protected:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
  UStaticMeshComponent *ObjectMesh;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
  EBuildableType ObjectType = EBuildableType::Decoration;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
  FString ObjectName = TEXT("Buildable Object");

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
  int32 BuildCost = 10;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
  bool bCanRotate = true;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
  FVector SnapOffset = FVector::ZeroVector;

public:
  // ========== Getters ==========

  UFUNCTION(BlueprintPure, Category = "Building")
  EBuildableType GetObjectType() const { return ObjectType; }

  UFUNCTION(BlueprintPure, Category = "Building")
  FString GetObjectName() const { return ObjectName; }

  UFUNCTION(BlueprintPure, Category = "Building")
  int32 GetBuildCost() const { return BuildCost; }

  // ========== Preview Mode ==========

  UFUNCTION(BlueprintCallable, Category = "Building")
  void SetPreviewMode(bool bIsPreview);

  UFUNCTION(BlueprintCallable, Category = "Building")
  void SetValidPlacement(bool bValid);

protected:
  virtual void CreateDefaultMesh();
};
