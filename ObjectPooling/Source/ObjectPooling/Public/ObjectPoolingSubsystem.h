// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "ObjectPoolingSubsystem.generated.h"


UCLASS()
class OBJECTPOOLING_API UObjectPoolingSubsystem : public UWorldSubsystem, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()
	// Define a tuple for the pool, containing arrays for active and inactive actors
	using Pool = TTuple<TArray<AActor*>, TArray<AActor*>>;

public:
#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	AActor* AcquireActorFromPool(TSubclassOf<AActor> Actor);

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void ReturnActorToPool(AActor* Actor);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

private:
	void AddActorToPool(AActor* Actor);
	void PoolInitialActors();

	// Map to store pools, keyed by actor class type
	TMap<TSubclassOf<AActor>, Pool> Pools;
};
