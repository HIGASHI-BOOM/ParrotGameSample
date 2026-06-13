// 2024 Secret Dimension, Inc.


#include "Enemy/ParrotEnemyPatrolRigComponent.h"

#include "Enemy/ParrotPatrolRigDebugVisualizer.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogPatrolRig);

// We are only allowed to create default subobjects as part of the constructor.
// However, at this point we cannot actually attach them - This occurs before attachment can be configured.
// We instead save off the handles so we can setup their attachment on OnComponentCreated.
UParrotEnemyPatrolRigComponent::UParrotEnemyPatrolRigComponent()
{
	PatrolSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PatrolSpline"));
	PatrolTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PatrolTriggerVolume"));
	AttackTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackTriggerVolume"));
#if WITH_EDITORONLY_DATA
	Visualizer = CreateDefaultSubobject<UParrotPatrolRigDebugVisualizer>(TEXT("PatrolVisualizer"));
#endif

	/** Here we have to set the default collision profile we want to make sure we aren't
	* checking collision against object types we don't care about.
	*
	* Normally we would set this in a blueprint base class, but since this is a native C++ class
	* we have to set it directly on our primitive components.
	*/
	PatrolTriggerBox->SetCollisionProfileName("OverlapAllPlayers");
	AttackTriggerBox->SetCollisionProfileName("OverlapAllPlayers");
}

// The only thing we need to do in begin play is to spawn the enemy and setup it's patrol.
// The default subobjects we create upon attachment don't need any setup here.
void UParrotEnemyPatrolRigComponent::BeginPlay()
{
	Super::BeginPlay();

	if (auto* Owner = GetOwner())
	{
		if (auto* World = Owner->GetWorld())
		{

			bool HasPatrolPoints = PatrolSpline->GetNumberOfSplinePoints() > 0;
			bool HasValidPatrol = PatrolSpline->GetNumberOfSplinePoints() > 1;

			if (!HasPatrolPoints)
			{
				UE_LOG(LogPatrolRig, Error, TEXT("Patrol Rig  on actor '%s' is missing patrol points! "
					"Have you authored points on the patrol spline ?"), 
					*GetAttachParent()->GetOwner()->GetName());
			}
			else if (!HasValidPatrol)
			{
				UE_LOG(LogPatrolRig, Warning, TEXT("Patrol Rig '%s' only has a single patrol point. "
					"The enemy will spawn at this point and not patrol. "
					"Did you mean to add more points?"), 
					*GetAttachParent()->GetOwner()->GetName());
			}

			// Get the location of the first spline in the patrol, this is where we will spawn the enemy.
			FTransform SpawnTransform = HasPatrolPoints ?
				FTransform(PatrolSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World)) :
				GetComponentTransform();

			if (HasValidPatrol)
			{
				// Get the rotation of this patrol point to the next patrol point so the enemy spawns facing the correct direction
				SpawnTransform.SetRotation(
					(PatrolSpline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World)
						- PatrolSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World))
					.ToOrientationQuat());
			}

			// This is a special deferred actor spawning pattern available in Unreal
			// It allows getting a handle to a spawned actor before it is initialized
			// so you can perform setup that must be done before the actor "starts up"
			//
			// Here we are passing in our init args, which has the patrol information, so when the enemy
			// is initialized during FinishSpawning(), the data is present
			if (auto* Enemy = World->SpawnActorDeferred<AParrotEnemyCharacterBase>(EnemyToSpawn.Get(), SpawnTransform, 
				nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
			{
				EnemyInitArgs.PatrolSpline = PatrolSpline;
				EnemyInitArgs.PatrolTriggerBox = PatrolTriggerBox;
				EnemyInitArgs.AttackTriggerBox = AttackTriggerBox;
				Enemy->Init(EnemyInitArgs);
				Enemy->FinishSpawning(SpawnTransform);
			}
		}
	}
}

// This is the appropriate place where we can setup subobject attachment to us.
// This places them in our parent actor's hierarchy so they inherit appropriate transforms
// and can be manipulated in the level editor.
void UParrotEnemyPatrolRigComponent::OnComponentCreated()
{	
	Super::OnComponentCreated();
	PatrolSpline->SetupAttachment(this);
	PatrolTriggerBox->SetupAttachment(this);
#if WITH_EDITORONLY_DATA
	Visualizer->SetupAttachment(this);
#endif
}

// The lifetime of default subobjects we create is tied to our lifetime - Since they are attached to us,
// they have to be manually destroyed when we, as their root component, are destroyed.
void UParrotEnemyPatrolRigComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (PatrolSpline != nullptr)
	{
		PatrolSpline->DestroyComponent();
		PatrolSpline = nullptr;
	}
	if (PatrolTriggerBox != nullptr)
	{
		PatrolTriggerBox->DestroyComponent();
		PatrolTriggerBox = nullptr;
	}
#if WITH_EDITORONLY_DATA
	if (Visualizer != nullptr)
	{
		Visualizer->DestroyComponent();
		Visualizer = nullptr;
	}
#endif

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
