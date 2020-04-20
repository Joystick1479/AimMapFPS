// Bartosz Jastrzebski


#include "RedEndgame.h"

#include "Components/SphereComponent.h"

#include "PayloadCharacter.h"

// Sets default values
ARedEndgame::ARedEndgame()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));

}

// Called when the game starts or when spawned
void ARedEndgame::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARedEndgame::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APayloadCharacter* Payload = Cast<APayloadCharacter>(OtherActor);
	if (Payload)
	{
		RedWins = true;
	}
}

void ARedEndgame::NotifyActorEndOverlap(AActor * OtherActor)
{
	APayloadCharacter* Payload = Cast<APayloadCharacter>(OtherActor);
	if (Payload)
	{
		RedWins = false;
	}
}

// Called every frame
void ARedEndgame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

