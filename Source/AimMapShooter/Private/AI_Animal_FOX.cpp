// Bartosz Jastrzebski


#include "AI_Animal_FOX.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

#include "SoldierCharacter.h"

// Sets default values
AAI_Animal_FOX::AAI_Animal_FOX()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HearingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HearingSphere"));
	HearingSphere->SetupAttachment(this->GetMesh());


}

// Called when the game starts or when spawned
void AAI_Animal_FOX::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAI_Animal_FOX::Hearing()
{
	TArray<AActor*> Target;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierChar, Target);
	for (int i = 0; i < Target.Num(); i++)
	{
		ASoldierCharacter* SoldChar = Cast<ASoldierCharacter>(Target[i]);
		if (SoldChar)
		{
			IsAttacking = HearingSphere->IsOverlappingActor(SoldChar);
		}
		else
		{
			IsAttacking = false;
		}
	}
}

// Called every frame
void AAI_Animal_FOX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Hearing();

	if (IsAttacking == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Atakuje"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NIE Atakuje"));
	}

}

// Called to bind functionality to input
void AAI_Animal_FOX::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

