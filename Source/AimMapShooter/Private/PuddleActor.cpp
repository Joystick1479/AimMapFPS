// Bartosz Jastrzebski


#include "PuddleActor.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
APuddleActor::APuddleActor()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = MeshComp;

}

// Called when the game starts or when spawned
void APuddleActor::BeginPlay()
{
	Super::BeginPlay();
	
}


