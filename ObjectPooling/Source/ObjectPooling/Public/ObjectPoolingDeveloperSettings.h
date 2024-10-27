// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ObjectPoolingDeveloperSettings.generated.h"

USTRUCT(BlueprintType)
struct FSubClassToPool
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AActor> Class;
	UPROPERTY(EditAnywhere)
	int32 InitialCount{0};
};

UCLASS(config=Game,defaultconfig, meta = (DisplayName="Object Pooling"))
class OBJECTPOOLING_API UObjectPoolingDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,Config)
	TArray<FSubClassToPool> InitialPools;
};
