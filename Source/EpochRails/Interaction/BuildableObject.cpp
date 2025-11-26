#include "Interaction/BuildableObject.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildable, Log, All);

ABuildableObject::ABuildableObject() {
  PrimaryActorTick.bCanEverTick = false;

  // Root - это меш объекта
  ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
  RootComponent = ObjectMesh;

  // Настройки коллизии
  ObjectMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  ObjectMesh->SetCollisionProfileName(TEXT("BlockAll"));

  CreateDefaultMesh();
}

void ABuildableObject::BeginPlay() {
  Super::BeginPlay();

  UE_LOG(LogBuildable, Log, TEXT("BuildableObject '%s' spawned (Type: %d)"),
         *ObjectName, static_cast<int32>(ObjectType));
}

void ABuildableObject::CreateDefaultMesh() {
  // Загружаем базовый куб из Engine Content
  static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
      TEXT("/Engine/BasicShapes/Cube"));

  if (CubeMeshAsset.Succeeded()) {
    ObjectMesh->SetStaticMesh(CubeMeshAsset.Object);

    // Размер зависит от типа объекта
    switch (ObjectType) {
    case EBuildableType::Storage:
      ObjectMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.5f)); // Высокий ящик
      break;

    case EBuildableType::Turret:
      ObjectMesh->SetWorldScale3D(FVector(0.8f, 0.8f, 1.2f)); // Узкая башня
      break;

    case EBuildableType::Generator:
      ObjectMesh->SetWorldScale3D(FVector(1.2f, 1.2f, 1.0f)); // Широкий куб
      break;

    case EBuildableType::Workbench:
      ObjectMesh->SetWorldScale3D(FVector(1.5f, 0.8f, 0.8f)); // Плоский стол
      break;

    case EBuildableType::Furniture:
      ObjectMesh->SetWorldScale3D(FVector(0.6f, 0.6f, 0.8f)); // Маленький
      break;

    default:                                                  // Decoration
      ObjectMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f)); // Маленький куб
      break;
    }

    // Материал по умолчанию
    UMaterialInterface *BaseMaterial = LoadObject<UMaterialInterface>(
        nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

    if (BaseMaterial) {
      UMaterialInstanceDynamic *ObjectMat =
          UMaterialInstanceDynamic::Create(BaseMaterial, this);

      // Цвет зависит от типа
      FLinearColor ObjectColor;
      switch (ObjectType) {
      case EBuildableType::Storage:
        ObjectColor = FLinearColor(0.6f, 0.4f, 0.2f); // Коричневый
        break;
      case EBuildableType::Turret:
        ObjectColor = FLinearColor(0.3f, 0.3f, 0.3f); // Серый
        break;
      case EBuildableType::Generator:
        ObjectColor = FLinearColor(0.8f, 0.8f, 0.2f); // Желтый
        break;
      case EBuildableType::Workbench:
        ObjectColor = FLinearColor(0.5f, 0.3f, 0.2f); // Темно-коричневый
        break;
      case EBuildableType::Furniture:
        ObjectColor = FLinearColor(0.4f, 0.6f, 0.8f); // Голубой
        break;
      default:
        ObjectColor = FLinearColor(0.2f, 0.8f, 0.2f); // Зеленый
        break;
      }

      ObjectMat->SetVectorParameterValue(FName("Color"), ObjectColor);
      ObjectMesh->SetMaterial(0, ObjectMat);
    }
  } else {
    UE_LOG(LogBuildable, Error,
           TEXT("Failed to load cube mesh for BuildableObject"));
  }
}

// ========== Preview Mode ==========

void ABuildableObject::SetPreviewMode(bool bIsPreview) {
  if (!ObjectMesh) {
    return;
  }

  if (bIsPreview) {
    // В режиме превью:
    // - Полупрозрачный
    // - Без коллизий
    ObjectMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Делаем материал полупрозрачным
    if (UMaterialInstanceDynamic *Mat =
            Cast<UMaterialInstanceDynamic>(ObjectMesh->GetMaterial(0))) {
      Mat->SetScalarParameterValue(FName("Opacity"), 0.5f);
    }

    UE_LOG(LogBuildable, Verbose, TEXT("Object '%s' set to preview mode"),
           *ObjectName);
  } else {
    // В обычном режиме:
    // - Непрозрачный
    // - С коллизиями
    ObjectMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    if (UMaterialInstanceDynamic *Mat =
            Cast<UMaterialInstanceDynamic>(ObjectMesh->GetMaterial(0))) {
      Mat->SetScalarParameterValue(FName("Opacity"), 1.0f);
    }

    UE_LOG(LogBuildable, Verbose, TEXT("Object '%s' set to normal mode"),
           *ObjectName);
  }
}

void ABuildableObject::SetValidPlacement(bool bValid) {
  if (!ObjectMesh) {
    return;
  }

  // Меняем цвет превью в зависимости от валидности размещения
  UMaterialInstanceDynamic *Mat =
      Cast<UMaterialInstanceDynamic>(ObjectMesh->GetMaterial(0));

  if (!Mat) {
    // Создаем динамический материал, если его нет
    UMaterialInterface *BaseMaterial = ObjectMesh->GetMaterial(0);
    if (BaseMaterial) {
      Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
      ObjectMesh->SetMaterial(0, Mat);
    }
  }

  if (Mat) {
    if (bValid) {
      // Зеленый = можно разместить
      Mat->SetVectorParameterValue(FName("Color"),
                                   FLinearColor(0.2f, 0.8f, 0.2f, 0.5f));
    } else {
      // Красный = нельзя разместить
      Mat->SetVectorParameterValue(FName("Color"),
                                   FLinearColor(0.8f, 0.2f, 0.2f, 0.5f));
    }
  }
}
