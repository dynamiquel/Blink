// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsclepiusDamageMultipliers.h"
#include "AsclepiusImmunityState.h"
#include "AsclepiusLifeState.h"
#include "Components/ActorComponent.h"
#include "AsclepiusHealthComponent.generated.h"

/** Adds health to an Actor. Works with UE4's default damage system but not required. */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASCLEPIUS_API UAsclepiusHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAsclepiusHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnRegister() override;
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_ImmunityState, EditAnywhere, Category="Health")
	EAsclepiusImmunityState ImmunityState = EAsclepiusImmunityState::None;
	
	UPROPERTY(ReplicatedUsing=OnRep_MaxHP, EditAnywhere, Category="Health")
	float MaxHP = 100.f;

	UPROPERTY(Replicated, EditAnywhere, Category="Health")
	float RegenRate = 0.f;

	UPROPERTY(Replicated, EditAnywhere, Category="Health")
	float RegenDelay = 0.f;

	UPROPERTY(Replicated, EditAnywhere, Category="Health")
	float InitialHP = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, VisibleAnywhere, Category="Health")
	float CurrentHP = 0.f;

	// Note: There is currently no difference between Death and Downed. It is however you decide to use them.
	UPROPERTY(ReplicatedUsing=OnRep_LifeState, VisibleAnywhere, Category="Health")
	EAsclepiusLifeState LifeState = EAsclepiusLifeState::Alive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health",
		meta=(RequiredAssetDataTags = "RowStructure=FAsclepiusDamageMultipliers"))
	FDataTableRowHandle BoneDamageModel;

	UPROPERTY(EditAnywhere, Category="Health")
	bool bIgnoreFriendlyDamage = true;

	UPROPERTY(VisibleAnywhere, Replicated, Category="Health")
	AController* LastDamageInstigator = nullptr;
	
protected:
	UFUNCTION()
	void OnRep_MaxHP() const;

	UFUNCTION()
	void OnRep_CurrentHP() const;

	UFUNCTION()
	void OnRep_ImmunityState() const;

	UFUNCTION()
	void OnRep_LifeState() const;

public:
	/**
	* @brief Inflicts damage.
	* @param Damage Amount of damage to inflict.
	* @param DamageType
	* @param LeftOverDamage Amount of damage left over.
	* @param Instigator Controller that is responsible for the damage.
	* @return Inflicted damage
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	float InflictDamage(const float Damage, float& LeftOverDamage, const UDamageType* DamageType = nullptr, AController* Instigator = nullptr);

	// When you can't be arsed to deal with LeftOverDamage.
	float InflictDamage(const float Damage, const UDamageType* DamageType = nullptr, AController* Instigator = nullptr);

	/**
	* @return True if this entity is dead.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	bool IsDead() const { return LifeState == EAsclepiusLifeState::Dead; }

	/**
	* @return Whether this entity is currently immune from all damage.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	bool IsImmune() const { return ImmunityState != EAsclepiusImmunityState::None; }

	UFUNCTION(BlueprintCallable, Category="Health")
	float GetBoneDamageMultiplier(const FName& BoneName);

	UFUNCTION(BlueprintCallable, Category="Health")
	bool GetIgnoreFriendlyDamage() const { return bIgnoreFriendlyDamage; }

// Events
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsclepiusOnDeathSignature,
		const UAsclepiusHealthComponent*, Sender);
	
	/**
	* float: New Capacity,
	* float: Damage Taken,
	* bool: Was Heal
	*/
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAsclepiusOnDamageSignature,
		const UAsclepiusHealthComponent*, Sender,
		const float, DamageInflicted,
		const float, NewCurrentHP,
		const UDamageType*, DamageType);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsclepiusOnImmunityStateChangedSignature,
		const UAsclepiusHealthComponent*, Sender,
		const EAsclepiusImmunityState, NewImmunityState);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAsclepiusOnHPChangedSignature,
		const UAsclepiusHealthComponent*, Sender,
		const float, NewCurrentHP,
		const float, NewMaxHP);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAsclepiusOnHPGainedSignature,
		const UAsclepiusHealthComponent*, Sender,
		const float, HealthGained,
		const float, NewCurrentHP);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsclepiusOnLifeStateChangedSignature,
		const UAsclepiusHealthComponent*, Sender,
		const EAsclepiusLifeState, NewLiveState);

	/**
	* @brief Triggered when this entity has taken damage through InflictDamage().
	*/
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category="Health")
	FAsclepiusOnDamageSignature OnDamage;

	/**
	* @brief Triggered when this entity has fully recovered their Health.
	*/
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category="Health")
	FAsclepiusOnDeathSignature OnHPFullyRecovered;
	
	/**
	* @brief Triggered when this entity's current Health has been changed.
	*
	* Server: Triggered by SetCurrentHealth and SetHealthCapacity.
	*
	* Client: Triggered by OnRep_CurrentHealth and OnRep_HealthCapacity.
	*/
	UPROPERTY(BlueprintAssignable, Category="Health")
	FAsclepiusOnHPChangedSignature OnHPChanged;

	/**
	* @brief Triggered when this entity's immunity state has been changed.
	*
	* Server: Triggered by SetImmunityState.
	*
	* Client: Triggered by OnRep_ImmunityState and OnRep_HealthCapacity.
	*/
	UPROPERTY(BlueprintAssignable, Category="Health")
	FAsclepiusOnImmunityStateChangedSignature OnImmunityStateChanged;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category="Health")
	FAsclepiusOnHPGainedSignature OnHealed;

	/**
	* @brief Triggered when this entity's live state has been changed.
	*
	* Server: Triggered by SetLiveState.
	*
	* Client: Triggered by OnRep_LiveState.
	*/
	UPROPERTY(BlueprintAssignable, Category="Health")
	FAsclepiusOnLifeStateChangedSignature OnLifeStateChanged;

protected:
	FTimerHandle HealthRegenHandle;
	void RegenerateHealth(float DeltaTime);

	/**
	 * @brief Tells the entity to die.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void Kill();

	/**
	 * @brief Tells the entity to go down.
	 */
	UFUNCTION(BlueprintCallable, Category="Health")
	void Incapacitate();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void DelayRegen();
	

	// Called when the entity has ran out of health.
	void HandleZeroHP();
	
// Getters and setters
public:
	/**
	* @return Whether this entity is currently immune from all damage, as well as which type of immunity it is.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	EAsclepiusImmunityState GetImmunityState() const { return ImmunityState; }

	/**
	* @brief Should this entity be immune from all damage?
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetImmunityState(const EAsclepiusImmunityState NewImmunityState);

	/**
	* @return Gets the life state of this entity.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	EAsclepiusLifeState GetLifeState() const { return LifeState; }

	/**
	* @brief Set the life state of this entity. This does not have a state machine behind it so manually changing
	* the state won't do anything. It just gives info on how to handle future events (may make it a state machine
	* in the future).
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetLifeState(const EAsclepiusLifeState NewLifeState);

	/**
	 * @brief Gets the max HP this Actor can get under normal conditions.
	 */
	UFUNCTION(BlueprintCallable, Category="Health")
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetMaxHP(float NewMaxHP);

	/**
	* @brief Gets how much HP is restored to the Actor per second.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	float GetRegenRate() const { return RegenRate; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetRegenRate(float NewRegenRate);

	/**
	* @brief Gets the time in seconds it takes for health regeneration to start after taking damage.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	float GetRegenDelay() const { return RegenDelay; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetRegenDelay(float NewRegenDelay);

	/**
	* @brief Gets the HP that this Actor started with.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	float GetInitialHP() const { return InitialHP; }

	/**
	* @brief Gets the current HP of this Actor.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetCurrentHP(float NewCurrentHP, const bool bAllowOverflow = false);

	/**
	* @brief Sets the current HP of this entity without any other processing such as death checks.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetCurrentHP_NoChecks(float NewCurrentHP);

	/**
	* @brief Gets the current health regen delay. Useful for HUD.
	*/
	UFUNCTION(BlueprintCallable, Category="Health")
	float GetCurrentRegenDelay() const;

	/**
	* @brief Gets the the Controller that dealt the most recent damage. Use during Dead state to check who killed this
	* Actor.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Health")
	AController* GetLastDamageInstigator() const { return LastDamageInstigator; }

protected:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void SetLastDamageInstigator(AController* NewDamageInstigator);

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void Resurrect(float NewCurrentHP = -1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void Revive(float NewCurrentHP = -1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Health")
	void FillHP();

	/**
	* @brief The state machine behind SetLifeState.
	* NewLifeState is equivalent to LifeState.
	*/
	UFUNCTION()
	void HandleLifeStateChanged(const UAsclepiusHealthComponent* Sender, const EAsclepiusLifeState NewLifeState);
};