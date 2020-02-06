// Bartosz Jastrzebski


#include "GrenadeDecal.h"

#include "Components/DecalComponent.h"

// Sets default values
AGrenadeDecal::AGrenadeDecal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	RootComponent = Decal;

}

// Called when the game starts or when spawned
void AGrenadeDecal::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrenadeDecal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

