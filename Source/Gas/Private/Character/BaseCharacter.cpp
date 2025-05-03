#include "Character/BaseCharacter.h"

#include "LyraCameraComponent.h"
#include "SignificanceManager.h"
#include "ReplicationGraph/SharedRepMovement.h"
#include "Character/Components/BaseCharacterMovementComponent.h"

#include "Character/Components/HealthComponent.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Character/Components/PawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"
#include "Tags/BaseGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseCharacter)
static FName NAME_CharacterCollisionProfile_Capsule(TEXT("PawnCapsule"));
static FName NAME_CharacterCollisionProfile_Mesh(TEXT("PawnMesh"));

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(CharacterMovementComponentName))
{
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Square of the max distance from the client's viewpoint that this actor is relevant and will be replicated.
	SetNetCullDistanceSquared(900000000.0f); // 3000 * 3000

	const auto CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_CharacterCollisionProfile_Capsule);

	const auto MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_CharacterCollisionProfile_Mesh);

	const auto MoveComp = GetCharacterMovement();
	check(MoveComp);
	MoveComp->GravityScale = 1.0f;
	MoveComp->MaxAcceleration = 2400.0f;
	MoveComp->BrakingFrictionFactor = 1.0f;
	MoveComp->BrakingFriction = 6.0f;
	MoveComp->GroundFriction = 8.0f;
	MoveComp->BrakingDecelerationWalking = 1400.0f;
	MoveComp->bUseControllerDesiredRotation = false;
	MoveComp->bOrientRotationToMovement = false;
	MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	MoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->SetCrouchedHalfHeight(65.0f);


	PawnExtComponent = CreateDefaultSubobject<UPawnExtensionComponent>("PawnExtensionComponent");
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	CameraComponent = CreateDefaultSubobject<ULyraCameraComponent>("CameraComponent");
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

ABasePlayerController* ABaseCharacter::GetBasePlayerController() const
{
	return CastChecked<ABasePlayerController>(Controller, ECastCheckedType::NullAllowed);
}

ABasePlayerState* ABaseCharacter::GetBasePlayerState() const
{
	return CastChecked<ABasePlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UBaseAbilitySystemComponent* ABaseCharacter::GetBaseAbilitySystemComponent() const { return Cast<UBaseAbilitySystemComponent>(GetAbilitySystemComponent()); }

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	if (PawnExtComponent)
		return PawnExtComponent->GetBaseAbilitySystemComponent();

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
		if (const auto SignificanceManager = USignificanceManager::Get<USignificanceManager>(World))
		{
			//@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
		}
	}
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	const UWorld* World = GetWorld();

	if (IsNetMode(NM_DedicatedServer))
		return;

	if (USignificanceManager* SignificanceManager = USignificanceManager::Get<USignificanceManager>(World))
		SignificanceManager->UnregisterObject(this);
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
	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
}

void ABaseCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
		return;

	// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
	const double MaxAccel = MovementComponent->MaxAcceleration;
	const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
	double AccelXYRadians, AccelXYMagnitude;
	FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

	ReplicatedAcceleration.AccelXYRadians = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0); // [0, 2PI] -> [0, 255]
	ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0); // [0, MaxAccel] -> [0, 255]
	ReplicatedAcceleration.AccelZ = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0); // [-MaxAccel, MaxAccel] -> [-127, 127]
}

void ABaseCharacter::NotifyControllerChanged()
{
}

#pragma endregion

#pragma region IGameplayTagAssetInterface
void ABaseCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent())
		Asc->GetOwnedGameplayTags(TagContainer);
}

bool ABaseCharacter::HasMatchingGameplayTag(const FGameplayTag TagToCheck) const
{
	const UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent();
	if (!Asc)
		return false;
	return Asc->HasMatchingGameplayTag(TagToCheck);
}

bool ABaseCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	const UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent();
	if (!Asc)
		return false;
	return Asc->HasAllMatchingGameplayTags(TagContainer);
}

bool ABaseCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	const UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent();
	if (!Asc)
		return false;
	return Asc->HasAnyMatchingGameplayTags(TagContainer);
}

#pragma endregion

bool ABaseCharacter::UpdateSharedReplication()
{
	// We cannot fast-rep right now. Don't send anything.
	if (GetLocalRole() != ROLE_Authority)
		return false;

	FSharedRepMovement SharedMovement;
	if (!SharedMovement.FillForCharacter(this))
		return false;

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
		return;

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() != ROLE_SimulatedProxy)
		return;

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
	UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent();
	check(Asc);

	HealthComponent->InitializeWithAbilitySystem(Asc);

	InitializeGameplayTags();
}

void ABaseCharacter::OnAbilitySystemUninitialized() { HealthComponent->UninitializeFromAbilitySystem(); }

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
	UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent();
	check(Asc);

	// Clear tags that may be lingering on the ability system from the previous pawn.
	for (const TPair<uint8, FGameplayTag>& TagMapping : MovementTags::MovementModeTagMap)
	{
		if (TagMapping.Value.IsValid())
			Asc->SetLooseGameplayTagCount(TagMapping.Value, 0);
	}

	for (const TPair<uint8, FGameplayTag>& TagMapping : MovementTags::CustomMovementModeTagMap)
	{
		if (TagMapping.Value.IsValid())
			Asc->SetLooseGameplayTagCount(TagMapping.Value, 0);
	}

	// Set the default movement mode tag.
	UCharacterMovementComponent* BaseMoveComp = GetCharacterMovement();
	SetMovementModeTag(BaseMoveComp->MovementMode, BaseMoveComp->CustomMovementMode, true);
}

void ABaseCharacter::FellOutOfWorld(const UDamageType& DmgType)
{
	HealthComponent->DamageSelfDestruct(true);
}

void ABaseCharacter::OnDeathStarted(AActor* OwningActor)
{
	DisableMovementAndCollision();
}

void ABaseCharacter::OnDeathFinished(AActor* OwningActor)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this,
	                                                  &ThisClass::DestroyDueToDeath);
}

void ABaseCharacter::DisableMovementAndCollision() const
{
	if (Controller)
		Controller->SetIgnoreMoveInput(true);

	const auto CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Stop movement and disable movement.
	const auto BaseMoveComp = GetCharacterMovement();
	BaseMoveComp->StopMovementImmediately();
	BaseMoveComp->DisableMovement();
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
	const auto Asc = GetBaseAbilitySystemComponent();
	if (Asc && Asc->GetAvatarActor() == this)
		PawnExtComponent->UninitializeAbilitySystem();

	SetActorHiddenInGame(true);
}

void ABaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	const UCharacterMovementComponent* BaseMoveComp = GetCharacterMovement();
	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(BaseMoveComp->MovementMode, BaseMoveComp->CustomMovementMode, true);
}

void ABaseCharacter::SetMovementModeTag(const EMovementMode MovementMode,
                                        const uint8 CustomMovementMode,
                                        const bool bTagEnabled) const
{
	const auto Asc = GetBaseAbilitySystemComponent();
	if (!Asc)
		return;

	const FGameplayTag* MovementModeTag;
	if (MovementMode == MOVE_Custom)
		MovementModeTag = MovementTags::CustomMovementModeTagMap.Find(CustomMovementMode);
	else
		MovementModeTag = MovementTags::MovementModeTagMap.Find(MovementMode);

	if (MovementModeTag && MovementModeTag->IsValid())
		Asc->SetLooseGameplayTagCount(*MovementModeTag, bTagEnabled ? 1 : 0);
}

void ABaseCharacter::ToggleCrouch()
{
	const auto MoveComp = GetCharacterMovement();

	if (bIsCrouched || MoveComp->bWantsToCrouch)
		UnCrouch();
	else if (MoveComp->IsMovingOnGround())
		Crouch();
}

void ABaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent())
		Asc->SetLooseGameplayTagCount(StatusTags::CROUCHING, 1);

	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent())
		Asc->SetLooseGameplayTagCount(StatusTags::CROUCHING, 0);

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool ABaseCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

void ABaseCharacter::OnRep_ReplicatedAcceleration() const
{
	const auto MovementComponent = Cast<UBaseCharacterMovementComponent>(GetCharacterMovement());
	if (!MovementComponent)
		return;

	// Decompress Acceleration
	const double MaxAccel = MovementComponent->MaxAcceleration;
	const double AccelXYMagnitude = static_cast<double>(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
	const double AccelXYRadians = static_cast<double>(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0; // [0, 255] -> [0, 2PI]

	FVector UnpackedAcceleration(FVector::ZeroVector);
	FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
	UnpackedAcceleration.Z = static_cast<double>(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

	MovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
}
