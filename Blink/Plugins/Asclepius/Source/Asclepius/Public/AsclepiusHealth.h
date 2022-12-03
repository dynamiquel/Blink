// © 2021 Liam Hall

#pragma once

#include "CoreMinimal.h"

#include "AsclepiusHealthComponent.h"
#include "UObject/Interface.h"
#include "AsclepiusHealth.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAsclepiusHealth : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class ASCLEPIUS_API IAsclepiusHealth
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Health)
	UAsclepiusHealthComponent* GetHealthComponent();
};