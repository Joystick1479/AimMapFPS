// Bartosz Jastrzebski


#include "BaseSurvivalItemClass.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ABaseSurvivalItemClass::ABaseSurvivalItemClass()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);

	SetReplicates(true);

}

// Called when the game starts or when spawned
void ABaseSurvivalItemClass::BeginPlay()
{
	Super::BeginPlay();

}

bool ABaseSurvivalItemClass::GetIfPickeditem()
{
	return bPickedUp;
}
void ABaseSurvivalItemClass::SetIfPickedItem(bool PickedUp)
{
	bPickedUp = PickedUp;
}

UStaticMeshComponent* ABaseSurvivalItemClass::GetMeshComponent()
{
	return MeshComp;
}
USphereComponent* ABaseSurvivalItemClass::GetSphereComponent()
{
	return SphereComp;
}

