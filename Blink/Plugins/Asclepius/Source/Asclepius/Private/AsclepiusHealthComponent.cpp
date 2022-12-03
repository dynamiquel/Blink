// Fill out your copyright notice in the Description page of Project Settings.


#include "AsclepiusHealthComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values for this component's properties
UAsclepiusHealthComponent::UAsclepiusHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// Auto-links the default DataTable. Just makes life easier for designers that don't know what they're doing.
	BoneDamageModel.DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Asclepius/DT_AsclepiusBoneDamageModels"));
	// Auto-assigns the 'NormalHuman' modifiers to this entity since this will be the most often used.
	BoneDamageModel.RowName = TEXT("NormalHuman");
	// ...
}

// Called when the game starts
void UAsclepiusHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	if (GetOwner()->HasAuthority())
		SetCurrentHP(GetInitialHP());	
}

void UAsclepiusHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, ImmunityState, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, LifeState, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, MaxHP, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, RegenRate, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, RegenDelay, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, InitialHP, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, CurrentHP, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( UAsclepiusHealthComponent, LastDamageInstigator, SharedParams );
}

// Called every frame
void UAsclepiusHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...

	if (!GetOwner()->HasAuthority())
		return;

	// Don't regen if dead.
	if (IsDead())
		return;

	// Health isn't on a delay.
	if (!GetWorld()->GetTimerManager().IsTimerActive(HealthRegenHandle))
		RegenerateHealth(DeltaTime);
}

void UAsclepiusHealthComponent::OnRegister()
{
	Super::OnRegister();
	
	if (!IsValid(GetWorld()) || !GetWorld()->IsGameWorld())
		return;

	// Self-subscribe.
	OnLifeStateChanged.AddUniqueDynamic(this, &UAsclepiusHealthComponent::HandleLifeStateChanged);
}

void UAsclepiusHealthComponent::OnRep_MaxHP() const
{
	OnHPChanged.Broadcast(this, CurrentHP, MaxHP);
}

void UAsclepiusHealthComponent::OnRep_CurrentHP() const
{
	OnHPChanged.Broadcast(this, CurrentHP, MaxHP);
}

void UAsclepiusHealthComponent::OnRep_ImmunityState() const
{
	OnImmunityStateChanged.Broadcast(this, ImmunityState);
}

void UAsclepiusHealthComponent::OnRep_LifeState() const
{
	OnLifeStateChanged.Broadcast(this, LifeState);
}

float UAsclepiusHealthComponent::InflictDamage(const float Damage, float& LeftOverDamage, const UDamageType* DamageType, AController* Instigator)
{
	LeftOverDamage = Damage;
	
	if (!GetOwner()->HasAuthority())
		return 0.f;

	// Ignore if no damage is being dealt.
	if (Damage == 0.f)
		return 0.f;

	if (IsDead())
		return 0.f;

	// Ignore damage if immunity is enabled.
	// TODO: Change this in future to allow certain damage types to pass through.
	if (IsImmune() && Damage > 0)
		return 0.f;

	// Do stuff

	const float PreviousCurrentHealthCapacity = GetCurrentHP();
	SetCurrentHP(PreviousCurrentHealthCapacity - Damage);

	LeftOverDamage = ((PreviousCurrentHealthCapacity - Damage) - GetCurrentHP()) * -1;
	const float InflictedDamage = Damage - LeftOverDamage;
	
	// Took damage.
	if (InflictedDamage > 0)
	{
		SetLastDamageInstigator(Instigator);
		OnDamage.Broadcast(this, InflictedDamage, CurrentHP, DamageType);
		DelayRegen();
	}
	// Gained health.
	else
	{
		// Negate the inflicted damage to redefine it as HP gained.
		OnHealed.Broadcast(this, InflictedDamage * -1, CurrentHP);
	}

	return InflictedDamage;
}

float UAsclepiusHealthComponent::InflictDamage(const float Damage, const UDamageType* DamageType, AController* Instigator)
{
	float LeftoverDamage;
	return InflictDamage(Damage, OUT LeftoverDamage, DamageType, Instigator);
}

float UAsclepiusHealthComponent::GetBoneDamageMultiplier(const FName& BoneName)
{
	if (!BoneDamageModel.IsNull())
	{
		FAsclepiusDamageMultipliers* FoundModel =
		BoneDamageModel.GetRow<FAsclepiusDamageMultipliers>(TEXT("Asclepius Health Bone Damage Multipliers"));

		if (FoundModel)
			return FoundModel->GetDamageMultipliers(BoneName);
	}
	
	return 1.f;
}

void UAsclepiusHealthComponent::RegenerateHealth(float DeltaTime)
{
	// Health regeneration is disabled.
	if (GetRegenRate() == 0)
		return;

	// Don't regen if full on health.
	if (GetCurrentHP() >= GetMaxHP())
		return;
		
	const float HealthToAdd = RegenRate * DeltaTime;
	InflictDamage(HealthToAdd * -1);

	// Health is now full.
	if (GetCurrentHP() >= GetMaxHP())
		OnHPFullyRecovered.Broadcast(this);
}

void UAsclepiusHealthComponent::Kill()
{
	SetLifeState(EAsclepiusLifeState::Dead);
}

void UAsclepiusHealthComponent::Incapacitate()
{
	SetLifeState(EAsclepiusLifeState::Downed);
}

void UAsclepiusHealthComponent::DelayRegen()
{
	if (!GetOwner()->HasAuthority())
		return;

	// No point setting a timer.
	if (RegenDelay <= 0.f)
		return;
	
	GetWorld()->GetTimerManager().SetTimer(HealthRegenHandle, RegenDelay, false);
}

void UAsclepiusHealthComponent::HandleZeroHP()
{
	// Currently there is no downed or final stand state, so just go straight to death.
	Kill();
}

void UAsclepiusHealthComponent::SetImmunityState(const EAsclepiusImmunityState NewImmunityState)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (ImmunityState == NewImmunityState)
		return;

	MARK_PROPERTY_DIRTY_FROM_NAME( UAsclepiusHealthComponent, ImmunityState, this );
	ImmunityState = NewImmunityState;

	OnImmunityStateChanged.Broadcast(this, ImmunityState);
}

void UAsclepiusHealthComponent::SetLifeState(const EAsclepiusLifeState NewLifeState)
{
	if (!GetOwner()->HasAuthority())
		return;

	// Life state didn't change.
	if (LifeState == NewLifeState)
		return;
	
	MARK_PROPERTY_DIRTY_FROM_NAME( UAsclepiusHealthComponent, LifeState, this );
	LifeState = NewLifeState;
	
	OnLifeStateChanged.Broadcast(this, LifeState);
}

void UAsclepiusHealthComponent::SetMaxHP(float NewMaxHP)
{
	if (!GetOwner()->HasAuthority())
		return;

	MARK_PROPERTY_DIRTY_FROM_NAME( UAsclepiusHealthComponent, MaxHP, this );
	MaxHP = NewMaxHP;

	OnHPChanged.Broadcast(this, CurrentHP, MaxHP);
}

void UAsclepiusHealthComponent::SetRegenRate(float NewRegenRate)
{
	if (!GetOwner()->HasAuthority())
		return;

	MARK_PROPERTY_DIRTY_FROM_NAME( UAsclepiusHealthComponent, RegenRate, this );
	RegenRate = NewRegenRate;
}

void UAsclepiusHealthComponent::SetRegenDelay(float NewRegenDelay)
{
	if (!GetOwner()->HasAuthority())
		return;

	MARK_PROPERTY_DIRTY_FROM_NAME( UAsclepiusHealthComponent, RegenDelay, this );
	RegenDelay = NewRegenDelay;
}

void UAsclepiusHealthComponent::SetCurrentHP(float NewCurrentHP, const bool bAllowOverflow)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (CurrentHP == NewCurrentHP)
		return;

	if (GetLifeState() != EAsclepiusLifeState::Alive)
		return;

	// If overflow isn't enabled, then don't make CurrentHP go over MaxHP.
	if (bAllowOverflow)
		NewCurrentHP = UKismetMathLibrary::FMax(NewCurrentHP, 0);
	else
		NewCurrentHP = UKismetMathLibrary::FClamp(NewCurrentHP, 0, MaxHP);

	SetCurrentHP_NoChecks(NewCurrentHP);

	if (CurrentHP == 0)
		HandleZeroHP();
}

void UAsclepiusHealthComponent::SetCurrentHP_NoChecks(float NewCurrentHP)
{
	if (!GetOwner()->HasAuthority())
		return;

	MARK_PROPERTY_DIRTY_FROM_NAME( UAsclepiusHealthComponent, CurrentHP, this );
	CurrentHP = NewCurrentHP;

	OnHPChanged.Broadcast(this, CurrentHP, MaxHP);
}

float UAsclepiusHealthComponent::GetCurrentRegenDelay() const
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(HealthRegenHandle))
		return 0.f;

	return GetWorld()->GetTimerManager().GetTimerRemaining(HealthRegenHandle);
}

void UAsclepiusHealthComponent::SetLastDamageInstigator(AController* NewDamageInstigator)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (GetLastDamageInstigator() == NewDamageInstigator)
		return;

	MARK_PROPERTY_DIRTY_FROM_NAME( UAsclepiusHealthComponent, LastDamageInstigator, this );
	LastDamageInstigator = NewDamageInstigator;
}

void UAsclepiusHealthComponent::Resurrect(float NewCurrentHP)
{
	if (!GetOwner()->HasAuthority())
		return;

	// Can't resurrect if alive.
	if (GetLifeState() == EAsclepiusLifeState::Alive)
		return;

	// Actor is downed, use Revive instead of Resurrect.
	if (GetLifeState() == EAsclepiusLifeState::Downed)
	{
		Revive(NewCurrentHP);
		return;
	}
	
	SetLifeState(EAsclepiusLifeState::Alive);
	// Use the value in InitialHP unless an overridable HP value is given.
	SetCurrentHP(NewCurrentHP >= 0.f ? NewCurrentHP : InitialHP);

	// Maybe RPC event?
}

void UAsclepiusHealthComponent::Revive(float NewCurrentHP)
{
	if (!GetOwner()->HasAuthority())
		return;

	// Can only revive if downed.
	if (GetLifeState() != EAsclepiusLifeState::Downed)
		return;

	SetLifeState(EAsclepiusLifeState::Alive);
	// Use the value in InitialHP unless an overridable HP value is given.
	SetCurrentHP(NewCurrentHP >= 0.f ? NewCurrentHP : InitialHP);

	// Maybe RPC event?
}

void UAsclepiusHealthComponent::FillHP()
{
	SetCurrentHP_NoChecks(GetMaxHP());
}

void UAsclepiusHealthComponent::HandleLifeStateChanged(const UAsclepiusHealthComponent* Sender,
                                                       const EAsclepiusLifeState NewLifeState)
{
	if (NewLifeState != EAsclepiusLifeState::Alive)
	{
		// If this is a forceful death/down (health didn't naturally reach 0, set it to 0).
		if (GetOwner()->HasAuthority())
		{
			if (CurrentHP > 0)
				SetCurrentHP_NoChecks(0);
		}
	}
}

