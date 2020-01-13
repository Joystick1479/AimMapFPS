// Bartosz Jastrzebski


#include "PayloadCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values
APayloadCharacter::APayloadCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp2"));
	PhysicsComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsComp"));

	UCapsuleComponent* CapsuleComp = this->GetCapsuleComponent();
	if (CapsuleComp)
	{
		MeshComp->SetupAttachment(CapsuleComp);
		MeshComp2->SetupAttachment(CapsuleComp);
		PhysicsComp->SetupAttachment(CapsuleComp);
	}

}

// Called when the game starts or when spawned
void APayloadCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APayloadCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APayloadCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

