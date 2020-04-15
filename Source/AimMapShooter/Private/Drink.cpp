// Bartosz Jastrzebski


#include "Drink.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ADrink::ADrink()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);

}

// Called when the game starts or when spawned
void ADrink::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADrink::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

