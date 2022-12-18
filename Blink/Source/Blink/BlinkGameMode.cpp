// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlinkGameMode.h"
#include "BlinkCharacter.h"
#include "BlinkGameState.h"
#include "BlinkPlayerController.h"
#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"

ABlinkGameMode::ABlinkGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	PlayerControllerClass = ABlinkPlayerController::StaticClass();
	GameStateClass = ABlinkGameState::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ABlinkGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update the time that enemy spawning has been active.
	if (GetWorldTimerManager().IsTimerActive(NextEnemySpawnTimer))
		GetGameState<ABlinkGameState>()->EnemySpawnTime += DeltaSeconds;
}

void ABlinkGameMode::EnemyKilled(int32 EnemyValue)
{
	// Reimburse the bullet cost.
	EnemyValue += BulletCost;
	
	GetGameState<ABlinkGameState>()->SetScore(GetGameState<ABlinkGameState>()->GetScore() + EnemyValue);
	GetGameState<ABlinkGameState>()->CurrentEnemies--;
}

void ABlinkGameMode::PlayerDied()
{
	GetGameState<ABlinkGameState>()->Deaths++;
	
	UKismetSystemLibrary::PrintString(this,
		FString::Printf(TEXT("Player Died (%d times)"), GetGameState<ABlinkGameState>()->Deaths));

	UGameplayStatics::OpenLevel(this, TEXT("FirstPersonMap"));
}

void ABlinkGameMode::PlayerBlinked()
{
	const int Blinks = ++GetGameState<ABlinkGameState>()->Blinks;

	UKismetSystemLibrary::PrintString(this, TEXT("Blinked"));

	// Keep on screen.
	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("Blinked %d/%d times"), Blinks, BlinksAllowed),
		true,
		false,
		FLinearColor(0, 1.f, .66f),
		FLT_MAX,
		FName(TEXT("BlinkNum")));
	
	// Player has blinked too many times.
	if (Blinks >= BlinksAllowed)
		PlayerDied();
}

void ABlinkGameMode::PlayerFound()
{
	if (!GetGameState<ABlinkGameState>()->bHasStarted)
	{
		// Re-enable player input.
		const auto PC = UGameplayStatics::GetPlayerController(this, 0);
		PC->GetPawn()->EnableInput(PC);

		StartEnemySpawning();
		GetGameState<ABlinkGameState>()->bHasStarted = true;
	}
}

void ABlinkGameMode::PlayerLost()
{
	// Classify a lost connection as a death.
	if (GetGameState<ABlinkGameState>()->bHasStarted)
	{
		PlayerDied();
	}
}

void ABlinkGameMode::BeginPlay()
{
	Super::BeginPlay();

	FindEnemySpawnPoints();
}

void ABlinkGameMode::StartEnemySpawning()
{
	// Get the Spawn Delay from the Float Curve, based on the total enemy spawning time.
	float SpawnDelay = SpawnRate.GetRichCurve()->Eval(GetGameState<ABlinkGameState>()->EnemySpawnTime);
	// Add the random time offset.
	SpawnDelay += FMath::FRandRange(0.f, SpawnDelay * RandomSpawnRateMultiplier);
	
	GetWorldTimerManager().SetTimer(
		NextEnemySpawnTimer,
		this,
		&ABlinkGameMode::SpawnNextEnemy,
		SpawnDelay);
}

void ABlinkGameMode::StopEnemySpawning()
{
	GetWorldTimerManager().ClearTimer(NextEnemySpawnTimer);
}

void ABlinkGameMode::GunFired() const
{
	// Costs 1 score to fire a bullet.
	GetGameState<ABlinkGameState>()->SetScore(GetGameState<ABlinkGameState>()->GetScore() - BulletCost);
}

void ABlinkGameMode::SpawnNextEnemy()
{
	// Skip if at max enemies at once.
	if (GetGameState<ABlinkGameState>()->CurrentEnemies < MaxEnemiesAtOnce)
	{
		// Choose an enemy type to spawn.
		const TSubclassOf<AActor> EnemyClass = ChooseEnemyClass();

		// Try spawn 3 times before giving up.
		for (int32 i = 0; i < 3; i++)
		{
			// Choose a spawn point for the enemy.
			const FTransform EnemySpawnTransform = ChooseEnemySpawnTransform(EnemyClass);

			AActor* SpawnedEnemy = GetWorld()->SpawnActorDeferred<AActor>(
				EnemyClass,
				EnemySpawnTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding);
		
			InitialiseSpawnedEnemy(SpawnedEnemy);		
			SpawnedEnemy->FinishSpawning(EnemySpawnTransform);

			// Only update variable if the Enemy spawn was successful (usually spawning collision issues).
			if (IsValid(SpawnedEnemy))
			{
				GetGameState<ABlinkGameState>()->CurrentEnemies++;
				break;
			}
		}
	}
	
	// Start timer again for next enemy spawn.
	StartEnemySpawning();
}

TSubclassOf<AActor> ABlinkGameMode::ChooseEnemyClass() const
{
	/*checkf(EnemyClasses.Num() > 0, TEXT("No Enemy Classes were added to the GameMode."));

	const int32 RandomIndex = FMath::RandRange(0, EnemyClasses.Num() - 1);
	return EnemyClasses[RandomIndex];*/

	ensureAlwaysMsgf(false, TEXT("ABlinkGameMode::ChooseEnemyClass has not been implemented."));

	return AEnemyCharacter::StaticClass();
}

void ABlinkGameMode::InitialiseSpawnedEnemy(AActor* Enemy)
{
}

void ABlinkGameMode::FindEnemySpawnPoints()
{
	// Find enemy spawn points using a tag.
	UGameplayStatics::GetAllActorsWithTag(this, FName(TEXT("EnemySpawn")), EnemySpawnPoints);
}

FTransform ABlinkGameMode::ChooseEnemySpawnTransform(const TSubclassOf<AActor> EnemyClass) const
{
	checkf(EnemySpawnPoints.Num() > 0, TEXT("No Enemy Spawn Points were found. Add the 'EnemySpawn' tag to spawn actors."));

	const int32 RandomIndex = FMath::RandRange(0, EnemySpawnPoints.Num() - 1);
	return EnemySpawnPoints[RandomIndex]->GetTransform();
}
