#include "Character/BaseCharacter.h"

#include "SignificanceManager.h"
#include "Character/SharedRepMovement.h"

#include "Character/Components/HealthComponent.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Character/Components/PawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"
#include "Tags/BaseGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseCharacter)

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Square of the max distance from the client's viewpoint that this actor is relevant and will be replicated.
	NetCullDistanceSquared = 900000000.0f; // 3000 * 3000

	PawnExtComponent = CreateDefaultSubobject<UPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
}

ABasePlayerController* ABaseCharacter::GetBasePlayerController() const
{
	return CastChecked<ABasePlayerController>(Controller, ECastCheckedType::NullAllowed);
}

ABasePlayerState* ABaseCharacter::GetBasePlayerState() const
{
	return CastChecked<ABasePlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UBaseAbilitySystemComponent* ABaseCharacter::GetBaseAbilitySystemComponent() const
{
	return Cast<UBaseAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	if (PawnExtComponent)
	{
		return PawnExtComponent->GetBaseAbilitySystemComponent();
	}

	return nullptr;
}

#pragma region Actor Interface
void ABaseCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	// UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();

	if (const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer))
	{
		if (USignificanceManager* SignificanceManager = USignificanceManager::Get<USignificanceManager>(World))
		{
			//@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
		}
	}
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const UWorld* World = GetWorld();

	if (!IsNetMode(NM_DedicatedServer))
	{
		if (USignificanceManager* SignificanceManager = USignificanceManager::Get<USignificanceManager>(World))
		{
			SignificanceManager->UnregisterObject(this);
		}
	}
}

void ABaseCharacter::Reset()
{
	DisableMovementAndCollision();
	K2_OnReset();
	UninitAndDestroy();
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ABaseCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
}

void ABaseCharacter::NotifyControllerChanged()
{
}

#pragma endregion

#pragma region IGameplayTagAssetInterface
void ABaseCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent())
	{
		BaseAsc->GetOwnedGameplayTags(TagContainer);
	}
}

bool ABaseCharacter::HasMatchingGameplayTag(const FGameplayTag TagToCheck) const
{
	const UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent();
	if (!BaseAsc)
	{
		return false;
	}
	return BaseAsc->HasMatchingGameplayTag(TagToCheck);
}

bool ABaseCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	const UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent();
	if (!BaseAsc)
	{
		return false;
	}
	return BaseAsc->HasAllMatchingGameplayTags(TagContainer);
}

bool ABaseCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	const UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent();
	if (!BaseAsc)
	{
		return false;
	}
	return BaseAsc->HasAnyMatchingGameplayTags(TagContainer);
}

#pragma endregion

bool ABaseCharacter::UpdateSharedReplication()
{
	// We cannot fast-rep right now. Don't send anything.
	if (GetLocalRole() != ROLE_Authority)
	{
		return false;
	}

	FSharedRepMovement SharedMovement;
	if (!SharedMovement.FillForCharacter(this))
	{
		return false;
	}

	// Only call FastSharedReplication if data has changed since the last frame.
	// Skipping this call will cause replication to reuse the same bunch that we previously
	// produced, but not send it to clients that already received. (But a new client who has not received
	// it, will get it this frame)
	if (!SharedMovement.Equals(LastSharedReplication, this))
	{
		LastSharedReplication = SharedMovement;
		ReplicatedMovementMode = SharedMovement.RepMovementMode;

		FastSharedReplication(SharedMovement);
	}
	return true;
}

void ABaseCharacter::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())
	{
		return;
	}

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() != ROLE_SimulatedProxy)
	{
		return;
	}

	// Timestamp
	ReplicatedServerLastTransformUpdateTimeStamp = SharedRepMovement.RepTimeStamp;

	// Movement mode
	if (ReplicatedMovementMode != SharedRepMovement.RepMovementMode)
	{
		ReplicatedMovementMode = SharedRepMovement.RepMovementMode;
		GetCharacterMovement()->bNetworkMovementModeChanged = true;
		GetCharacterMovement()->bNetworkUpdateReceived = true;
	}

	// Location, Rotation, Velocity, etc.
	FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
	MutableRepMovement = SharedRepMovement.RepMovement;

	// This also sets LastRepMovement
	OnRep_ReplicatedMovement();

	// Jump force
	bProxyIsJumpForceApplied = SharedRepMovement.bProxyIsJumpForceApplied;

	// Crouch
	if (bIsCrouched != SharedRepMovement.bIsCrouched)
	{
		bIsCrouched = SharedRepMovement.bIsCrouched;
		OnRep_IsCrouched();
	}
}

void ABaseCharacter::OnAbilitySystemInitialized()
{
	UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent();
	check(BaseAsc);

	HealthComponent->InitializeWithAbilitySystem(BaseAsc);

	InitializeGameplayTags();
}

void ABaseCharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();
}

void ABaseCharacter::UnPossessed()
{
	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();
}

void ABaseCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}

void ABaseCharacter::InitializeGameplayTags() const
{
	UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent();
	check(BaseAsc);

	// Clear tags that may be lingering on the ability system from the previous pawn.
	for (const TPair<uint8, FGameplayTag>& TagMapping : MovementTags::MovementModeTagMap)
	{
		if (TagMapping.Value.IsValid())
		{
			BaseAsc->SetLooseGameplayTagCount(TagMapping.Value, 0);
		}
	}

	for (const TPair<uint8, FGameplayTag>& TagMapping : MovementTags::CustomMovementModeTagMap)
	{
		if (TagMapping.Value.IsValid())
		{
			BaseAsc->SetLooseGameplayTagCount(TagMapping.Value, 0);
		}
	}
	// todo :implement this
	// Set the default movement mode tag.
	// UBaseCharacterMovementComponent* BaseMoveComp = CastChecked<UBaseCharacterMovementComponent>(GetCharacterMovement());
	// SetMovementModeTag(BaseMoveComp->MovementMode, BaseMoveComp->CustomMovementMode, true);
}

void ABaseCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
	HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/true);
}

void ABaseCharacter::OnDeathStarted(AActor* OwningActor)
{
	DisableMovementAndCollision();
}

void ABaseCharacter::OnDeathFinished(AActor* OwningActor)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}

void ABaseCharacter::DisableMovementAndCollision() const
{
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	// todo : implement this
	// Stop movement and disable movement.
	// UBaseCharacterMovementComponent* BaseMoveComp = CastChecked<UBaseCharacterMovementComponent>(GetCharacterMovement());
	// BaseMoveComp->StopMovementImmediately();
	// BaseMoveComp->DisableMovement();
}

void ABaseCharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();
	UninitAndDestroy();
}

void ABaseCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor
	// (otherwise another pawn already did it when they became the avatar actor)
	if (const UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent())
	{
		if (BaseAsc->GetAvatarActor() == this)
		{
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void ABaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	// UBaseCharacterMovementComponent* BaseMoveComp = CastChecked<UBaseCharacterMovementComponent>(GetCharacterMovement());
	//
	// SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	// SetMovementModeTag(BaseMoveComp->MovementMode, BaseMoveComp->CustomMovementMode, true);
}

void ABaseCharacter::SetMovementModeTag(const EMovementMode MovementMode, const uint8 CustomMovementMode,
                                        const bool bTagEnabled) const
{
	UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent();
	if (!BaseAsc)
	{
		return;
	}

	const FGameplayTag* MovementModeTag;
	if (MovementMode == MOVE_Custom)
	{
		MovementModeTag = MovementTags::CustomMovementModeTagMap.Find(CustomMovementMode);
	}
	else
	{
		MovementModeTag = MovementTags::MovementModeTagMap.Find(MovementMode);
	}

	if (MovementModeTag && MovementModeTag->IsValid())
	{
		BaseAsc->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
	}
}

void ABaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent())
	{
		BaseAsc->SetLooseGameplayTagCount(StatusTags::CROUCHING, 1);
	}

	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent())
	{
		BaseAsc->SetLooseGameplayTagCount(StatusTags::CROUCHING, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool ABaseCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

void ABaseCharacter::OnRep_ReplicatedAcceleration() const
{
}
