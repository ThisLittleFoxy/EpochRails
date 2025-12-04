# Utils System (Система утилит)

## Описание

Вспомогательные сервисы для общих задач в системе EpochRails.

## Основные классы

### UAimTraceService
**Локация:** `Source/EpochRails/Utils/AimTraceService.h`

#### Функции

```cpp
// Выполнить трассировку от камеры
UFUNCTION(BlueprintCallable, Category = "Aim Trace")
static bool PerformAimTrace(
    const UObject* WorldContextObject,
    FVector Start,
    FVector Direction,
    float Distance,
    FHitResult& OutHit,
    ECC_WorldDynamic TraceChannel = ECC_Visibility,
    bool bDrawDebug = false
);

// Получить точку тычки для IK
UFUNCTION(BlueprintCallable, Category = "Aim Trace")
static FVector GetAimHitLocation(
    const UObject* WorldContextObject,
    APlayerController* Controller,
    float MaxDistance = 1000.0f
);

// Проверить тычку на тип актора
UFUNCTION(BlueprintCallable, Category = "Aim Trace")
static bool IsAimingAtActorOfClass(
    const UObject* WorldContextObject,
    APlayerController* Controller,
    TSubclassOf<AActor> ActorClass,
    float MaxDistance = 1000.0f
);
```

## Версия документации
- **EpochRails Version:** 0.0.22
- **Last Updated:** 2025-12-04
