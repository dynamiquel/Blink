// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlinkCharacter.h"
#include "BlinkProjectile.h"
#include "TP_PickUpComponent.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "AsclepiusHealth.h"
#include "BlinkGameMode.h"
#include "GameFramework/PlayerState.h"


//////////////////////////////////////////////////////////////////////////
// ABlinkCharacter

ABlinkCharacter::ABlinkCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(44.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateOptionalDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	// Since the comp is optional, need to check if it was created.
	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
		FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
		FirstPersonCameraComponent->bUsePawnControlRotation = true;
	}

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	if (Mesh1P)
	{
		Mesh1P->SetOnlyOwnerSee(true);
		Mesh1P->SetupAttachment(FirstPersonCameraComponent);
		Mesh1P->bCastDynamicShadow = false;
		Mesh1P->CastShadow = false;
		Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
		Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
	}

	// Create the health component.
	HealthComponent = CreateDefaultSubobject<UAsclepiusHealthComponent>(TEXT("Health"));
}

float ABlinkCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{	
	// Does all the damage modifier calculations.
	DamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	if (DamageAmount != 0 && IsValid(HealthComponent))
	{
		// Converts the given DamageType class reference to an object reference.
		UDamageType const* const DamageTypeCDO = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();

		float LeftOverDamage;
		// Sends the damage to the health component so it can do the rest of the damage modifier calculations and
		// set health/shields values.
		DamageAmount = HealthComponent->InflictDamage(DamageAmount, LeftOverDamage, DamageTypeCDO);
	}

	return DamageAmount;
}

void ABlinkCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	HealthComponent->OnLifeStateChanged.AddUniqueDynamic(this, &ABlinkCharacter::HandleLifeStateChanged);
}

float ABlinkCharacter::InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (HealthComponent)
	{		
		const float PrecisionDamageMultiplier = HealthComponent->GetBoneDamageMultiplier(PointDamageEvent.HitInfo.BoneName);
		Damage *= PrecisionDamageMultiplier;
	}
	
	return Damage;
}

//////////////////////////////////////////////////////////////////////////// Input

void ABlinkCharacter::HandleLifeStateChanged(const UAsclepiusHealthComponent* Sender,
	const EAsclepiusLifeState NewLiveState)
{
	switch (NewLiveState)
	{
		case EAsclepiusLifeState::Alive:
			break;
		case EAsclepiusLifeState::Downed:
			break;
		case EAsclepiusLifeState::Dead:
			// Destroy actor in 10 seconds.
			OnDeath();
			break;
	}
}

void ABlinkCharacter::OnDeath_Implementation()
{
	// Quick way of determining if enemy or not, cba doing it the proper way.
	if (GetController() && GetController()->IsPlayerController())
	{
		GetWorld()->GetAuthGameMode<ABlinkGameMode>()->PlayerDied();
	}
	else
	{
		GetWorld()->GetAuthGameMode<ABlinkGameMode>()->EnemyKilled();
	}
	
	FTimerHandle DestroyTimer;
	GetWorldTimerManager().SetTimer(DestroyTimer, FTimerDelegate::CreateLambda([this]()
	{
		if (IsValid(this))
			Destroy();
	}), 0.1f, false);
}

void ABlinkCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &ABlinkCharacter::OnPrimaryAction);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ABlinkCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ABlinkCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ABlinkCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ABlinkCharacter::LookUpAtRate);
}

void ABlinkCharacter::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	OnUseItem.Broadcast();
}

void ABlinkCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ABlinkCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void ABlinkCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ABlinkCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ABlinkCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABlinkCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

bool ABlinkCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ABlinkCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ABlinkCharacter::EndTouch);

		return true;
	}
	
	return false;
}
