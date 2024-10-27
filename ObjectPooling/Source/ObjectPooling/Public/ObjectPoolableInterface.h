// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ObjectPoolableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UObjectPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class OBJECTPOOLING_API IObjectPoolableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//To set up the object when it is first created or retrieved from the pool.
	UFUNCTION(BlueprintNativeEvent)
	void Initialize();
	//To reset the object's state before it is returned to the pool, allowing you to clear or reinitialize any properties.
	UFUNCTION(BlueprintNativeEvent)
	void Reset();
};
