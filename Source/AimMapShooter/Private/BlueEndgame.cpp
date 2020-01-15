// Bartosz Jastrzebski


#include "BlueEndgame.h"

#include "Components/SphereComponent.h"

#include "PayloadCharacter.h"


// Sets default values
ABlueEndgame::ABlueEndgame()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));


}

// Called when the game starts or when spawned
void ABlueEndgame::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlueEndgame::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APayloadCharacter* Payload = Cast<APayloadCharacter>(OtherActor);
	if (Payload)
	{
		BlueWins = true;
		UE_LOG(LogTemp, Warning, TEXT("Blue wins, true"));
	}
}

void ABlueEndgame::NotifyActorEndOverlap(AActor * OtherActor)
{
	APayloadCharacter* Payload = Cast<APayloadCharacter>(OtherActor);
	if (Payload)
	{
		BlueWins = false;
	}
}

// Called every frame
void ABlueEndgame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

