#include "ObjectPoolingSubsystem.h"

#include "ObjectPoolableInterface.h"
#include "ObjectPoolingDeveloperSettings.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(LogPoolingSubsystem, Log, All);

void UObjectPoolingSubsystem::PoolInitialActors()
{
	const UObjectPoolingDeveloperSettings* Settings = GetDefault<UObjectPoolingDeveloperSettings>();
	ensure(Settings);

	for (const FSubClassToPool& PoolEntry : Settings->InitialPools)
	{
		TSubclassOf<AActor> ActorClass = PoolEntry.Class.LoadSynchronous();
		int32 NumberToPool = PoolEntry.InitialCount;

		if (!ensure(ActorClass)) continue;
		if (!ensure(NumberToPool > 0)) continue;
		
		for (int32 i = 0; i < NumberToPool; ++i)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
			AActor* PooledActor = GetWorld()->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
			if (!ensure(PooledActor)) continue;

			if (PooledActor->Implements<UObjectPoolableInterface>())
			{
				IObjectPoolableInterface::Execute_Reset(PooledActor);
			}
		
			AddActorToPool(PooledActor);
		}
	}
}

#if ENABLE_VISUAL_LOG
void UObjectPoolingSubsystem::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	if (!Snapshot)
	{
		return;
	}
	
	const int32 CatIndex = Snapshot->Status.AddZeroed();
	FVisualLogStatusCategory& Category = Snapshot->Status[CatIndex];
	Category.Category = TEXT("Object Pooling");

	for (const auto& Pair : Pools)
	{
		TSubclassOf<AActor> ActorClass = Pair.Key;
		const Pool& ActorPool = Pair.Value;

		const TArray<AActor*>& ActiveActors = ActorPool.Get<0>();
		const TArray<AActor*>& InactiveActors = ActorPool.Get<1>();

		int32 ActiveCount = ActiveActors.Num();
		int32 InactiveCount = InactiveActors.Num();

		FString ActorClassName = ActorClass ? ActorClass->GetName() : TEXT("Unknown");
        
		Category.Add(TEXT("Pooling"), FString::Printf(TEXT("%s: Active: %d, Inactive: %d"), *ActorClassName, ActiveCount, InactiveCount));
	}
}
#endif

AActor* UObjectPoolingSubsystem::AcquireActorFromPool(TSubclassOf<AActor> Actor)
{
	if (!ensure(Actor)) return nullptr;

	if (Pools.Contains(Actor))
	{
		Pool& ActorPool = Pools[Actor];

		TArray<AActor*>& InactiveActors = ActorPool.Get<1>();
		TArray<AActor*>& ActiveActors = ActorPool.Get<0>();

		if (InactiveActors.Num() > 0)
		{
			AActor* PooledActor = InactiveActors.Pop();
			ActiveActors.Push(PooledActor);

			if (PooledActor->Implements<UObjectPoolableInterface>())
			{
				IObjectPoolableInterface::Execute_Initialize(PooledActor);
			}
			UE_VLOG(this, LogPoolingSubsystem, Log, TEXT("AcquireActorFromPool"));
			UE_LOGFMT(LogPoolingSubsystem, Log, "Acquired actor {a} from the pool.", PooledActor->GetName());
			return PooledActor;
		}
		else
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* NewActor = GetWorld()->SpawnActor<AActor>(Actor, FVector::ZeroVector, FRotator::ZeroRotator, Params);
			ensure(NewActor);

			AddActorToPool(NewActor);

			UE_LOGFMT(LogPoolingSubsystem, Log, "Spawned and added new actor {a} to the pool as none were available.", NewActor->GetName());
			return NewActor;
		}
	}

	UE_LOGFMT(LogPoolingSubsystem, Warning, "No pool found for actor class {a}.", Actor->GetName());
	return nullptr;
}

void UObjectPoolingSubsystem::ReturnActorToPool(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOGFMT(LogPoolingSubsystem, Warning, "Attempted to return a null actor to the pool.");
		return;
	}

	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	ensure(ActorClass);

	if (!Pools.Contains(ActorClass))
	{
		UE_LOGFMT(LogPoolingSubsystem, Warning, "No pool found for actor class {a}. Cannot return actor {b}.", ActorClass->GetName(), Actor->GetName());
		return;
	}

	Pool& ActorPool = Pools[ActorClass];

	TArray<AActor*>& ActiveActors = ActorPool.Get<0>();
	TArray<AActor*>& InactiveActors = ActorPool.Get<1>();

	if (ActiveActors.Remove(Actor) > 0)
	{
		if (Actor->Implements<UObjectPoolableInterface>())
		{
			IObjectPoolableInterface::Execute_Reset(Actor);
		}

		InactiveActors.Add(Actor);
        
		UE_LOGFMT(LogPoolingSubsystem, Log, "Returned actor {a} to the pool.", Actor->GetName());
	}
	else
	{
		UE_LOGFMT(LogPoolingSubsystem, Warning, "Actor {a} not found in the active pool.", Actor->GetName());
	}
}

void UObjectPoolingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOGFMT(LogPoolingSubsystem, Warning, "Initialize");
	UE_VLOG(this, LogPoolingSubsystem, Warning, TEXT("Initialize"));
}

void UObjectPoolingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UObjectPoolingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	PoolInitialActors();
}

bool UObjectPoolingSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	switch (WorldType)
	{
	case EWorldType::None:
		return false;
	case EWorldType::Game:
		return true;
	case EWorldType::Editor:
		return false;
	case EWorldType::PIE:
		return true;
	case EWorldType::EditorPreview:
		return false;
	case EWorldType::GamePreview:
		return true;
	case EWorldType::GameRPC:
		return true;
	case EWorldType::Inactive:
		return false;
	}

	return Super::DoesSupportWorldType(WorldType);
}

void UObjectPoolingSubsystem::AddActorToPool(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOGFMT(LogPoolingSubsystem, Warning, "Attempted to add a null actor to the pool.");
		return;
	}

	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	ensure(ActorClass);

	if (!Pools.Contains(ActorClass))
	{
		Pools.FindOrAdd(ActorClass, Pool(TArray<AActor*>{}, TArray{Actor}));
		UE_LOGFMT(LogPoolingSubsystem, Warning, "Created new pool for actor class: {a}", ActorClass->GetName());
		return;
	}

	Pool& ActorPool = Pools[ActorClass];

	TArray<AActor*>& ActiveActors = ActorPool.Get<0>();
	TArray<AActor*>& InactiveActors = ActorPool.Get<1>();

	if (ActiveActors.Remove(Actor) > 0)
	{
		if (Actor->Implements<UObjectPoolableInterface>())
		{
			IObjectPoolableInterface::Execute_Reset(Actor);
		}
        
		InactiveActors.Add(Actor);
        
		UE_LOGFMT(LogPoolingSubsystem, Log, "Actor {a} added to the pool.", Actor->GetName());
	}
	else
	{
		UE_LOGFMT(LogPoolingSubsystem, Warning, "Actor {a} not found in the active pool.", Actor->GetName());
	}
}