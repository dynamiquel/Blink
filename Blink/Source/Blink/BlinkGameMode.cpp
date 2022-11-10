// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlinkGameMode.h"
#include "BlinkCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABlinkGameMode::ABlinkGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
