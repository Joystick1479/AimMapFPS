// Bartosz Jastrzebski


#include "Rifle_3rd.h"

#include "Components/SkeletalMeshComponent.h"

// Sets default values
ARifle_3rd::ARifle_3rd()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMeshComp"));
	RootComponent = SkelMeshComp;


}

// Called when the game starts or when spawned
void ARifle_3rd::BeginPlay()
{
	Super::BeginPlay();
	
}


USkeletalMeshComponent* ARifle_3rd::GetSkelMeshComp()
{
	return this->SkelMeshComp;
}
