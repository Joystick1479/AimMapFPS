// Bartosz Jastrzebski


#include "Payload.h"

#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


// Sets default values
APayload::APayload()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp2"));
	RootMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMeshComp"));
	PhysicsComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsComp"));

	RootComponent = RootMeshComp;
	
	MeshComp->SetupAttachment(RootMeshComp);
	MeshComp2->SetupAttachment(RootMeshComp);
	PhysicsComp->SetupAttachment(RootMeshComp);

}

// Called when the game starts or when spawned
void APayload::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APayload::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

