// Bartosz Jastrzebski


#include "Drink.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "SoldierCharacter.h"

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

void ADrink::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bDrinkPickup = true;
	}

}

void ADrink::NotifyActorEndOverlap(AActor * OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bDrinkPickup = false;
	}

}

// Called every frame
void ADrink::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsPickedUp == true)
	{
		this->Destroy();
	}
}

