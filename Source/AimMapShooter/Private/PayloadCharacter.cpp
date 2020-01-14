// Bartosz Jastrzebski


#include "PayloadCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"

#include "SoldierCharacter.h"
#include "BlueEndgame.h"
#include "RedEndgame.h"

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
void APayloadCharacter::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{

		APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
				///CHECKING IF CONTROLLED PLAYER IS HOSTING ///
				FString CheckName = UKismetSystemLibrary::GetObjectName(PC);
				FString FindName = "PlayerController_0";
				UE_LOG(LogTemp, Warning, TEXT("Test"));
				if (0 == 0)
				{
					TArray<AActor*> BlueEndGame;
					UGameplayStatics::GetAllActorsOfClass(this, BlueEndgameClass, BlueEndGame);

					if (BlueEndGame.Num() > 0)
					{
						FVector BlueLocation = BlueEndGame[0]->GetActorLocation();
						UE_LOG(LogTemp, Warning, TEXT("Vector is : %s"), *BlueLocation.ToString());
						UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), BlueLocation);
					}
				}
				else
				{
					TArray<AActor*> RedEndGame;
					UGameplayStatics::GetAllActorsOfClass(this, RedEndgameClass, RedEndGame);
					if (RedEndGame.Num() > 0)
					{
						FVector RedLocation = RedEndGame[0]->GetActorLocation();
						UNavigationSystem::SimpleMoveToLocation(GetController(), RedLocation);
					}
				}
		}
	}
}
void APayloadCharacter::NotifyActorEndOverlap(AActor * OtherActor)
{

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		UCharacterMovementComponent* CharMovement = this->FindComponentByClass<UCharacterMovementComponent>();
		if (CharMovement)
		{
			CharMovement->StopActiveMovement();
		}
	}
}
