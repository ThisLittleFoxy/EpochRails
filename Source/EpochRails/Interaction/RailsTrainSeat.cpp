// Copyright Epic Games, Inc. All Rights Reserved.

#include "RailsTrainSeat.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/RailsPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "TimerManager.h"
#include "Train/RailsTrain.h"

ARailsTrainSeat::ARailsTrainSeat() {
  PrimaryActorTick.bCanEverTick = false;

  // Create root component
  SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
  RootComponent = SceneRoot;

  // Create mesh component for seat visual representation
  MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
  MeshComponent->SetupAttachment(RootComponent);

  // Create seat point component
  SeatPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SeatPoint"));
  SeatPoint->SetupAttachment(RootComponent);

  // Setup interaction properties
  InteractionName = FText::FromString(TEXT("Train Driver Seat"));
  InteractionAction = FText::FromString(TEXT("Sit"));
  bCanInteract = true;
  MaxInteractionDistance = 200.0f;
  bEnableDebugLog = false;

  // Initialize seat state
  bIsPlayerSeated = false;
  SeatedPlayer = nullptr;

  // Initialize animation flags
  bIsPlayingSitDownAnimation = false;
  bIsPlayingStandUpAnimation = false;

  // Animation settings
  bPlaySitDownAnimation = true;
  bPlayStandUpAnimation = true;
  bPlaySittingIdleAnimation = true;
  SeatAnimationPlayRate = 1.0f;

  // Seat behavior settings
  bHideCharacterWhileSeated = false;
  bAttachCharacterToSeat = true;
}

ARailsTrainSeat::~ARailsTrainSeat() {
  // Destructor - cleanup is handled in EndPlay
}

void ARailsTrainSeat::BeginPlay() { Super::BeginPlay(); }

void ARailsTrainSeat::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  // Clean up timers before destruction
  if (UWorld *World = GetWorld()) {
    if (FTimerManager *TimerManager = &World->GetTimerManager()) {
      // Clear all active timers
      if (SitDownAnimationTimerHandle.IsValid()) {
        TimerManager->ClearTimer(SitDownAnimationTimerHandle);
      }

      if (StandUpAnimationTimerHandle.IsValid()) {
        TimerManager->ClearTimer(StandUpAnimationTimerHandle);
      }
    }
  }

  // If player is seated, unseat them before destruction
  if (bIsPlayerSeated && SeatedPlayer) {
    // Stop animations
    StopSeatAnimations(SeatedPlayer);

    // Restore player controls without unseating animation
    ApplyInputMappingContexts(SeatedPlayer, false);

    // Clear references
    if (SeatedPlayer) {
      SeatedPlayer->SetControlledTrain(nullptr);
      SeatedPlayer->SetCurrentSeat(nullptr);
    }

    SeatedPlayer = nullptr;
    bIsPlayerSeated = false;
  }

  // Reset animation flags
  bIsPlayingSitDownAnimation = false;
  bIsPlayingStandUpAnimation = false;

  Super::EndPlay(EndPlayReason);
}

// ========== IInteractableInterface Implementation ==========

void ARailsTrainSeat::OnInteractionFocusBegin_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log, TEXT("%s: Player started looking at seat"),
           *GetName());
  }

  // Add visual feedback - highlight the seat mesh
  if (MeshComponent) {
    MeshComponent->SetRenderCustomDepth(true);
    MeshComponent->SetCustomDepthStencilValue(
        252); // Custom stencil value for outline
  }
}

void ARailsTrainSeat::OnInteractionFocusEnd_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log, TEXT("%s: Player stopped looking at seat"),
           *GetName());
  }

  // Remove visual feedback
  if (MeshComponent) {
    MeshComponent->SetRenderCustomDepth(false);
  }
}

bool ARailsTrainSeat::OnInteract_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter) {
    return false;
  }

  // Block interaction if currently playing animations
  if (bIsPlayingSitDownAnimation || bIsPlayingStandUpAnimation) {
    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Warning,
             TEXT("%s: Interaction blocked - animation in progress"),
             *GetName());
    }
    return false;
  }

  // Toggle seat state on interaction
  if (!bIsPlayerSeated) {
    // Start sitting down process
    if (bPlaySitDownAnimation && SitDownAnimationMontage) {
      PlaySitDownAnimation(PlayerCharacter);
    } else {
      // No animation, seat immediately
      SeatPlayer(PlayerCharacter);
    }
  } else {
    // If another player is already seated, only allow the same player to get up
    if (SeatedPlayer == PlayerCharacter) {
      // Start standing up process
      if (bPlayStandUpAnimation && StandUpAnimationMontage) {
        PlayStandUpAnimation(PlayerCharacter);
      } else {
        // No animation, unseat immediately
        UnseatPlayer();
      }
    } else {
      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Warning,
               TEXT("%s: Cannot interact - another player is seated"),
               *GetName());
      }
      return false;
    }
  }

  return true;
}

FText ARailsTrainSeat::GetInteractionName_Implementation() const {
  return InteractionName;
}

FText ARailsTrainSeat::GetInteractionAction_Implementation() const {
  // Change prompt depending on state
  if (bIsPlayerSeated) {
    return FText::FromString(TEXT("Stand up"));
  }

  return InteractionAction;
}

bool ARailsTrainSeat::CanInteract_Implementation(
    ARailsPlayerCharacter *PlayerCharacter) const {
  // Basic interaction check
  if (!bCanInteract) {
    return false;
  }

  // Block interaction while playing animations
  if (bIsPlayingSitDownAnimation || bIsPlayingStandUpAnimation) {
    return false;
  }

  // Require a valid train and train control IMC to be assigned
  if (!ControlledTrain || !TrainControlInputMappingContext) {
    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Warning,
             TEXT("%s: Cannot interact - missing train or input context"),
             *GetName());
    }
    return false;
  }

  // If someone else is already seated, block interaction
  if (bIsPlayerSeated && SeatedPlayer && SeatedPlayer != PlayerCharacter) {
    return false;
  }

  return true;
}

float ARailsTrainSeat::GetInteractionDistance_Implementation() const {
  return MaxInteractionDistance;
}

// ========== Animation Methods ==========

void ARailsTrainSeat::PlaySitDownAnimation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter || !SitDownAnimationMontage) {
    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Warning,
             TEXT("%s: Cannot play sit down animation - invalid parameters"),
             *GetName());
    }
    SeatPlayer(PlayerCharacter);
    return;
  }

  // Check if World is valid
  UWorld *World = GetWorld();
  if (!World) {
    UE_LOG(LogTemp, Error, TEXT("%s: Cannot play animation - World is invalid"),
           *GetName());
    SeatPlayer(PlayerCharacter);
    return;
  }

  // Cache player reference
  SeatedPlayer = PlayerCharacter;
  bIsPlayingSitDownAnimation = true;

  // Get character's animation instance
  UAnimInstance *AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
  if (!AnimInstance) {
    UE_LOG(LogTemp, Warning, TEXT("%s: Cannot get AnimInstance"), *GetName());
    bIsPlayingSitDownAnimation = false;
    SeatPlayer(PlayerCharacter);
    return;
  }

  // Play sit down animation
  float MontageLength = AnimInstance->Montage_Play(
      SitDownAnimationMontage, SeatAnimationPlayRate,
      EMontagePlayReturnType::MontageLength, 0.0f, true);

  if (MontageLength > 0.0f) {
    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Log, TEXT("%s: Playing sit down animation (%.2fs)"),
             *GetName(), MontageLength);
    }

    // Setup completion callback
    float ActualDuration = MontageLength / SeatAnimationPlayRate;
    World->GetTimerManager().SetTimer(
        SitDownAnimationTimerHandle, this,
        &ARailsTrainSeat::OnSitDownAnimationComplete, ActualDuration, false);
  } else {
    UE_LOG(LogTemp, Warning, TEXT("%s: Failed to play sit down animation"),
           *GetName());
    bIsPlayingSitDownAnimation = false;
    SeatPlayer(PlayerCharacter);
  }
}

void ARailsTrainSeat::OnSitDownAnimationComplete() {
  bIsPlayingSitDownAnimation = false;

  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log, TEXT("%s: Sit down animation completed"), *GetName());
  }

  // Clear timer first
  if (UWorld *World = GetWorld()) {
    World->GetTimerManager().ClearTimer(SitDownAnimationTimerHandle);
  }

  if (SeatedPlayer) {
    // Complete the seating process
    SeatPlayer(SeatedPlayer);

    // Note: SeatPlayer() now handles starting idle animation,
    // so we don't need to call PlaySittingIdleAnimation here anymore
  }
}

void ARailsTrainSeat::PlayStandUpAnimation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter || !StandUpAnimationMontage) {
    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Warning,
             TEXT("%s: Cannot play stand up animation - invalid parameters"),
             *GetName());
    }
    UnseatPlayer();
    return;
  }

  // Check if World is valid
  UWorld *World = GetWorld();
  if (!World) {
    UE_LOG(LogTemp, Error, TEXT("%s: Cannot play animation - World is invalid"),
           *GetName());
    UnseatPlayer();
    return;
  }

  bIsPlayingStandUpAnimation = true;

  // Stop sitting idle animation first
  StopSeatAnimations(PlayerCharacter);

  // Get character's animation instance
  UAnimInstance *AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
  if (!AnimInstance) {
    UE_LOG(LogTemp, Warning, TEXT("%s: Cannot get AnimInstance"), *GetName());
    bIsPlayingStandUpAnimation = false;
    UnseatPlayer();
    return;
  }

  // Play stand up animation
  float MontageLength = AnimInstance->Montage_Play(
      StandUpAnimationMontage, SeatAnimationPlayRate,
      EMontagePlayReturnType::MontageLength, 0.0f, true);

  if (MontageLength > 0.0f) {
    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Log, TEXT("%s: Playing stand up animation (%.2fs)"),
             *GetName(), MontageLength);
    }

    // Setup completion callback
    float ActualDuration = MontageLength / SeatAnimationPlayRate;
    World->GetTimerManager().SetTimer(
        StandUpAnimationTimerHandle, this,
        &ARailsTrainSeat::OnStandUpAnimationComplete, ActualDuration, false);
  } else {
    UE_LOG(LogTemp, Warning, TEXT("%s: Failed to play stand up animation"),
           *GetName());
    bIsPlayingStandUpAnimation = false;
    UnseatPlayer();
  }
}

void ARailsTrainSeat::OnStandUpAnimationComplete() {
  bIsPlayingStandUpAnimation = false;

  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log, TEXT("%s: Stand up animation completed"), *GetName());
  }

  // Clear timer first
  if (UWorld *World = GetWorld()) {
    World->GetTimerManager().ClearTimer(StandUpAnimationTimerHandle);
  }

  if (SeatedPlayer) {
    // Complete the unseating process
    UnseatPlayer();
  }
}

void ARailsTrainSeat::PlaySittingIdleAnimation(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter || !SittingIdleAnimationMontage) {
    return;
  }

  UAnimInstance *AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
  if (!AnimInstance) {
    return;
  }

  // Play looping idle animation
  float MontageLength = AnimInstance->Montage_Play(
      SittingIdleAnimationMontage, 1.0f, EMontagePlayReturnType::MontageLength,
      0.0f, true);

  if (MontageLength > 0.0f) {
    // Set to loop (if montage has proper loop setup)
    AnimInstance->Montage_SetNextSection(FName("Default"), FName("Default"),
                                         SittingIdleAnimationMontage);

    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Log, TEXT("%s: Playing sitting idle animation (looping)"),
             *GetName());
    }
  }
}

void ARailsTrainSeat::StopSeatAnimations(
    ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter) {
    return;
  }

  UAnimInstance *AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
  if (!AnimInstance) {
    return;
  }

  // Stop any playing montage
  if (AnimInstance->IsAnyMontagePlaying()) {
    AnimInstance->Montage_Stop(0.2f);

    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Log, TEXT("%s: Stopped all seat animations"), *GetName());
    }
  }
}

// ========== Seat Control Methods ==========

void ARailsTrainSeat::SeatPlayer(ARailsPlayerCharacter *PlayerCharacter) {
  if (!PlayerCharacter || !ControlledTrain) {
    UE_LOG(LogTemp, Error, TEXT("%s: Cannot seat player - invalid parameters"),
           *GetName());
    return;
  }

  SeatedPlayer = PlayerCharacter;
  bIsPlayerSeated = true;

  // Position player at seat point
  if (SeatPoint) {
    FVector SeatLocation = SeatPoint->GetComponentLocation();
    FRotator SeatRotation = SeatPoint->GetComponentRotation();

    if (bAttachCharacterToSeat) {
      // Attach character to seat (better for moving trains)
      PlayerCharacter->AttachToComponent(
          SeatPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Character attached to seat"),
               *GetName());
      }
    } else {
      // Simply teleport player to seat point
      PlayerCharacter->SetActorLocation(SeatLocation);
      PlayerCharacter->SetActorRotation(SeatRotation);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Character teleported to seat"),
               *GetName());
      }
    }
  }

  // Disable character movement while seated
  if (UCharacterMovementComponent *MovementComp =
          PlayerCharacter->GetCharacterMovement()) {
    MovementComp->DisableMovement();
  }

  // Optionally hide character mesh
  if (bHideCharacterWhileSeated) {
    if (USkeletalMeshComponent *MeshComp = PlayerCharacter->GetMesh()) {
      MeshComp->SetVisibility(false);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Character mesh hidden"), *GetName());
      }
    }
  }

  // Switch input mapping to train control - IMMEDIATELY ENABLE CONTROLS
  ApplyInputMappingContexts(PlayerCharacter, true);

  // Set controlled train reference
  PlayerCharacter->SetControlledTrain(ControlledTrain);
  PlayerCharacter->SetCurrentSeat(this);

  // IMPORTANT: Play sitting idle animation immediately if enabled
  if (bPlaySittingIdleAnimation && SittingIdleAnimationMontage) {
    PlaySittingIdleAnimation(PlayerCharacter);
  }

  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log,
           TEXT("%s: Player seated successfully - train controls ENABLED"),
           *GetName());
  }
}

void ARailsTrainSeat::UnseatPlayer() {
  if (!SeatedPlayer) {
    bIsPlayerSeated = false;
    return;
  }

  ARailsPlayerCharacter *PlayerCharacter = SeatedPlayer;

  // Stop all animations
  StopSeatAnimations(PlayerCharacter);

  // Detach character from seat if attached
  if (bAttachCharacterToSeat) {
    PlayerCharacter->DetachFromActor(
        FDetachmentTransformRules::KeepWorldTransform);

    if (bEnableDebugLog) {
      UE_LOG(LogTemp, Log, TEXT("%s: Character detached from seat"),
             *GetName());
    }
  }

  // Re-enable character movement
  if (UCharacterMovementComponent *MovementComp =
          PlayerCharacter->GetCharacterMovement()) {
    MovementComp->SetMovementMode(MOVE_Walking);
  }

  // Show character mesh if it was hidden
  if (bHideCharacterWhileSeated) {
    if (USkeletalMeshComponent *MeshComp = PlayerCharacter->GetMesh()) {
      MeshComp->SetVisibility(true);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Character mesh shown"), *GetName());
      }
    }
  }

  // Move player slightly away from seat to avoid re-triggering interaction
  FVector ExitLocation = PlayerCharacter->GetActorLocation() +
                         PlayerCharacter->GetActorForwardVector() * 100.0f;
  PlayerCharacter->SetActorLocation(ExitLocation);

  // Restore default input mapping - IMMEDIATELY DISABLE TRAIN CONTROLS
  ApplyInputMappingContexts(PlayerCharacter, false);

  // Clear controlled train and seat
  PlayerCharacter->SetControlledTrain(nullptr);
  PlayerCharacter->SetCurrentSeat(nullptr);

  SeatedPlayer = nullptr;
  bIsPlayerSeated = false;

  if (bEnableDebugLog) {
    UE_LOG(LogTemp, Log,
           TEXT("%s: Player unseated successfully - train controls DISABLED"),
           *GetName());
  }
}

// ========== Helper Methods ==========

void ARailsTrainSeat::ApplyInputMappingContexts(
    ARailsPlayerCharacter *PlayerCharacter, bool bUseTrainControl) {
  if (!PlayerCharacter) {
    return;
  }

  UEnhancedInputLocalPlayerSubsystem *InputSubsystem =
      GetInputSubsystem(PlayerCharacter);
  if (!InputSubsystem) {
    UE_LOG(LogTemp, Warning, TEXT("%s: Cannot get InputSubsystem"), *GetName());
    return;
  }

  if (bUseTrainControl) {
    // Remove default IMC if assigned
    if (DefaultInputMappingContext) {
      InputSubsystem->RemoveMappingContext(DefaultInputMappingContext);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Removed default input context"),
               *GetName());
      }
    }

    // Add train control IMC
    if (TrainControlInputMappingContext) {
      InputSubsystem->AddMappingContext(TrainControlInputMappingContext,
                                        TrainControlIMCPriority);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Added train control input context"),
               *GetName());
      }
    }
  } else {
    // Remove train control IMC
    if (TrainControlInputMappingContext) {
      InputSubsystem->RemoveMappingContext(TrainControlInputMappingContext);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Removed train control input context"),
               *GetName());
      }
    }

    // Restore default IMC
    if (DefaultInputMappingContext) {
      InputSubsystem->AddMappingContext(DefaultInputMappingContext,
                                        DefaultIMCPriority);

      if (bEnableDebugLog) {
        UE_LOG(LogTemp, Log, TEXT("%s: Restored default input context"),
               *GetName());
      }
    }
  }
}

UEnhancedInputLocalPlayerSubsystem *ARailsTrainSeat::GetInputSubsystem(
    ARailsPlayerCharacter *PlayerCharacter) const {
  if (!PlayerCharacter) {
    return nullptr;
  }

  APlayerController *PC =
      Cast<APlayerController>(PlayerCharacter->GetController());
  if (!PC) {
    return nullptr;
  }

  ULocalPlayer *LocalPlayer = PC->GetLocalPlayer();
  if (!LocalPlayer) {
    return nullptr;
  }

  return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
