# Unreal Engine Generic Object Pooling System

This repository provides a generic object pooling system developed in C++ for Unreal Engine. The goal is to optimize object creation and management by reducing the performance overhead caused by frequent actor spawning and despawning. The system supports different actor classes and reuses inactive objects instead of destroying them.

## Features

- **Object Pooling**: Reuse actors to improve performance.
- **Configurable via Settings**: Define initial pools and counts using `UObjectPoolingDeveloperSettings`.
- **Structured Logging with `UE_LOGFMT`**: Utilize structured logs for detailed information and warning messages.
- **Visual Logger Integration**: Track pool status and active/inactive actors using Unreal Engine's Visual Logger.

## Usage

### Prerequisites

- Unreal Engine 5.4( think will work with lower versions but not tested)

### Setup

1. **Add the Subsystem to Your Project**:
   - Include the `ObjectPoolingSubsystem.h` and `ObjectPoolingSubsystem.cpp` files in your Unreal project.

2. **Configure Pooled Classes**:
   - In the Unreal Editor, go to `Project Settings > ObjectPoolingDeveloperSettings`.
   - Add the actor classes you want to include in the pool and set the initial count for each class.

3. **Implement the `ObjectPoolableInterface` (Optional)**:
   - Create an interface that inherits from `UObjectPoolableInterface` to customize the behavior of objects when initialized and reset in the pool.

### Example Usage

In your gameplay code, you can acquire and return objects to the pool:

```cpp
// Acquire an actor from the pool
AActor* Actor = GetWorld()->GetSubsystem<UObjectPoolingSubsystem>()->AcquireActorFromPool(MyActorClass);

if (Actor)
{
    // Use the actor as needed
}

// Return an actor to the pool
GetWorld()->GetSubsystem<UObjectPoolingSubsystem>()->ReturnActorToPool(Actor);
