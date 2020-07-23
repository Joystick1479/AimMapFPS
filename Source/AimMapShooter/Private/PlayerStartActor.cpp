// Bartosz Jastrzebski


#include "PlayerStartActor.h"

#include "Components/SphereComponent.h"

#include "GameFramework/Character.h" 

APlayerStartActor::APlayerStartActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
}
void APlayerStartActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ACharacter * Character = Cast<ACharacter>(OtherActor);
	if (Character)
	{
		bIsOverlapping = true;
	}
}
void APlayerStartActor::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ACharacter * Character = Cast<ACharacter>(OtherActor);
	if (Character)
	{
		bIsOverlapping = false;
	}
}

bool APlayerStartActor::GetbIsOverlapping()
{
	return bIsOverlapping;
}
