// Bartosz Jastrzebski


#include "Magazine.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "TimerManager.h"

#include "Character/SoldierCharacter.h"

// Sets default values
AMagazine::AMagazine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);
}

// Called when the game starts or when spawned
void AMagazine::BeginPlay()
{
	Super::BeginPlay();

	DestroyOnUse();
	
}

void AMagazine::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bMagazinePickUp = true;
	}
}

void AMagazine::NotifyActorEndOverlap(AActor * OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		SoldierCharacter->bMagazinePickUp = false;
	}
}

void AMagazine::DestroyOnUse()
{
	if (IsPickedUp == true)
	{
		this->Destroy();
	}

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AMagazine::DestroyOnUse, 0.5f, false);

}

// Called every frame
void AMagazine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

