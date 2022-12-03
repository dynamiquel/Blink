#pragma once

#include "AsclepiusDamageMultiplierSet.generated.h"

USTRUCT(BlueprintType)
struct ASCLEPIUS_API FAsclepiusDamageMultiplierSet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSet<FName> BoneNames;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageMultiplier = 1.f;
};