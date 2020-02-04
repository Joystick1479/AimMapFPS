// Bartosz Jastrzebski


#include "FlashGrenade.h"

#include "Components/StaticMeshComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "SoldierCharacter.h"

// Sets default values
AFlashGrenade::AFlashGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

}

// Called when the game starts or when spawned
void AFlashGrenade::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Character;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierChar, Character);

	for (int i = 0; i < Character.Num(); i++)
	{
		ASoldierCharacter*New = Cast<ASoldierCharacter>(Character[i]);
		if (New)
		{

		}
	}
	
}

void AFlashGrenade::ThrowinGrenade(FVector Impulse, FVector Impulse2)
{
	
	Impulse = MeshComp->GetForwardVector() * 750;
	Impulse2 = UKismetMathLibrary::RandomUnitVector() * 500;

	MeshComp->AddImpulse(Impulse, NAME_None, true);
	MeshComp->AddAngularImpulseInDegrees(Impulse, NAME_None, true);
	
}

// Called every frame
void AFlashGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

