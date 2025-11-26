#include "Interaction/TrainControlPanel.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

#include "Character/RailsPlayerCharacter.h"
#include "Controllers/RailsPlayerController.h"
#include "Train/BaseVehicle.h"

DEFINE_LOG_CATEGORY_STATIC(LogTrainPanel, Log, All);

ATrainControlPanel::ATrainControlPanel() {
  PrimaryActorTick.bCanEverTick = false;

  // Root компонент
  USceneComponent *Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  RootComponent = Root;

  // Panel Mesh (корпус панели)
  PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
  PanelMesh->SetupAttachment(RootComponent);

  // Screen Mesh (экран)
  ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
  ScreenMesh->SetupAttachment(PanelMesh);
  ScreenMesh->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));

  // Widget Component (3D UI)
  ControlWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("ControlWidget"));
  ControlWidget->SetupAttachment(ScreenMesh);
  ControlWidget->SetWidgetSpace(EWidgetSpace::World);
  ControlWidget->SetDrawSize(FVector2D(400.0f, 300.0f));

  CreateDefaultMesh();
}

void ATrainControlPanel::CreateDefaultMesh() {
  // Загружаем базовый куб для панели
  static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
      TEXT("/Engine/BasicShapes/Cube"));

  if (CubeMeshAsset.Succeeded()) {
    // Корпус панели (серый куб 100x200x150)
    PanelMesh->SetStaticMesh(CubeMeshAsset.Object);
    PanelMesh->SetWorldScale3D(FVector(1.0f, 2.0f, 1.5f));
    PanelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PanelMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // Экран (черный куб 10x180x130)
    ScreenMesh->SetStaticMesh(CubeMeshAsset.Object);
    ScreenMesh->SetWorldScale3D(FVector(0.1f, 1.8f, 1.3f));
    ScreenMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Материалы
    UMaterialInterface *BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

    if (BaseMaterial) {
      // Серый корпус
      UMaterialInstanceDynamic *PanelMat =
          UMaterialInstanceDynamic::Create(BaseMaterial, this);
      PanelMat->SetVectorParameterValue(FName("Color"),
                                        FLinearColor(0.3f, 0.3f, 0.3f));
      PanelMesh->SetMaterial(0, PanelMat);

      // Черный экран
      UMaterialInstanceDynamic *ScreenMat =
          UMaterialInstanceDynamic::Create(BaseMaterial, this);
      ScreenMat->SetVectorParameterValue(FName("Color"),
                                         FLinearColor(0.05f, 0.05f, 0.1f));
      ScreenMesh->SetMaterial(0, ScreenMat);
    }
  }
}

void ATrainControlPanel::BeginPlay() {
  Super::BeginPlay();

  // Автоопределение владеющего поезда
  if (!OwningTrain) {
    AActor *OwnerActor = GetOwner();
    if (OwnerActor) {
      OwningTrain = Cast<ABaseVehicle>(OwnerActor);
      if (OwningTrain) {
        UE_LOG(LogTrainPanel, Log, TEXT("Auto-detected owning train: %s"),
               *OwningTrain->GetName());
      }
    }
  }
}

// ========== IInteractableInterface Implementation ==========

void ATrainControlPanel::OnLookAtStart_Implementation() {
  bIsHighlighted = true;
  UpdateHighlight(true);
  UE_LOG(LogTrainPanel, Verbose, TEXT("Panel highlighted"));
}

void ATrainControlPanel::OnLookAtEnd_Implementation() {
  bIsHighlighted = false;
  UpdateHighlight(false);
  UE_LOG(LogTrainPanel, Verbose, TEXT("Panel unhighlighted"));
}

void ATrainControlPanel::OnInteract_Implementation(
    AController *InstigatorController) {
  if (!OwningTrain) {
    UE_LOG(LogTrainPanel, Warning, TEXT("Cannot interact: No owning train"));
    return;
  }

  ARailsPlayerController *PC =
      Cast<ARailsPlayerController>(InstigatorController);
  if (!PC) {
    UE_LOG(LogTrainPanel, Warning,
           TEXT("Controller is not RailsPlayerController"));
    return;
  }

  ARailsPlayerCharacter *Character = PC->GetRailsCharacter();
  if (!Character) {
    UE_LOG(LogTrainPanel, Warning, TEXT("No character found"));
    return;
  }

  // Входим в режим управления поездом
  Character->EnterTrainControlMode(OwningTrain);
  PC->EnterTrainControlMode(OwningTrain);

  UE_LOG(LogTrainPanel, Log, TEXT("Player entered train control mode"));
}

FText ATrainControlPanel::GetInteractionText_Implementation() const {
  return InteractionPrompt;
}

bool ATrainControlPanel::CanInteract_Implementation(
    AController *InstigatorController) const {
  // Можно взаимодействовать только если есть поезд
  return OwningTrain != nullptr;
}

// ========== Setup ==========

void ATrainControlPanel::SetOwningTrain(ABaseVehicle *Train) {
  OwningTrain = Train;
  UE_LOG(LogTrainPanel, Log, TEXT("Owning train set to: %s"),
         Train ? *Train->GetName() : TEXT("None"));
}

void ATrainControlPanel::UpdateHighlight(bool bHighlight) {
  // Меняем цвет при наведении
  if (UMaterialInstanceDynamic *Mat =
          Cast<UMaterialInstanceDynamic>(PanelMesh->GetMaterial(0))) {
    if (bHighlight) {
      Mat->SetVectorParameterValue(FName("Color"),
                                   FLinearColor(0.5f, 0.5f, 0.8f)); // Синеватый
    } else {
      Mat->SetVectorParameterValue(FName("Color"),
                                   FLinearColor(0.3f, 0.3f, 0.3f)); // Серый
    }
  }

  // Подсветка экрана
  if (UMaterialInstanceDynamic *ScreenMat =
          Cast<UMaterialInstanceDynamic>(ScreenMesh->GetMaterial(0))) {
    if (bHighlight) {
      ScreenMat->SetVectorParameterValue(
          FName("Color"), FLinearColor(0.1f, 0.3f, 0.5f)); // Светлее
    } else {
      ScreenMat->SetVectorParameterValue(
          FName("Color"), FLinearColor(0.05f, 0.05f, 0.1f)); // Темный
    }
  }
}
