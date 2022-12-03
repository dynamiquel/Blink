#pragma once

#include "AsclepiusDamageMultiplierSet.h"
#include "Engine/DataTable.h"
#include "AsclepiusDamageMultipliers.generated.h"


USTRUCT()
struct ASCLEPIUS_API FAsclepiusDamageMultipliers : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DefaultDamageMultiplier = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAsclepiusDamageMultiplierSet> CustomDamageMultipliers;

	float GetDamageMultipliers(const FName& BoneName) const
	{
		for (FAsclepiusDamageMultiplierSet ModifierSet : CustomDamageMultipliers)
		{
			if (ModifierSet.BoneNames.Contains(BoneName))
				return ModifierSet.DamageMultiplier;
		}

		return DefaultDamageMultiplier;
	}
};