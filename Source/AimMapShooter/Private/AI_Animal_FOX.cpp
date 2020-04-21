// Bartosz Jastrzebski


#include "AI_Animal_FOX.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "HealthComponent.h"

#include "TimerManager.h"

#include "SoldierCharacter.h"

#include "AI_Animal_Controller.h"

#include "Net/UnrealNetwork.h"


// Sets default values
AAI_Animal_FOX::AAI_Animal_FOX()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HearingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HearingSphere"));
	HearingSphere->SetupAttachment(this->GetMesh());

	DamagingSphere = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DamagingSphere"));
	DamagingSphere->SetupAttachment(this->GetMesh());

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	SetReplicates(true);
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
			if (IsAttacking == true)
			{
				UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
				if (MoveComp)
				{
					MoveComp->MaxWalkSpeed = 600;
				}
			}
			else if (IsAttacking == false)
			{
				UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
				if (MoveComp)
				{
					MoveComp->MaxWalkSpeed = 150;
				}
			}
		}
		else
		{
			//IsAttacking = false;
		}
	}
}

void AAI_Animal_FOX::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
		{
			if (MoveComp)
			{
				MoveComp->DisableMovement();
				MoveComp->StopActiveMovement();
				MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
			}
		}
	}
}

void AAI_Animal_FOX::NotifyActorEndOverlap(AActor * OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(OtherActor);
	if (SoldierCharacter)
	{
		IsAttacking = false;
		UE_LOG(LogTemp, Warning, TEXT("NieAtakuje"));
	}
}

void AAI_Animal_FOX::Attacking()
{

	TArray<AActor*> Target;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierChar, Target);
	for (int i = 0; i < Target.Num(); i++)
	{
		ASoldierCharacter* SoldChar = Cast<ASoldierCharacter>(Target[i]);
		if (SoldChar)
		{
			if (DamagingSphere->IsOverlappingActor(SoldChar) == true)
			{
				//OpenGate
				if (DoOnce == false)
				{
					UGameplayStatics::ApplyDamage(SoldChar, 30.0f, nullptr, nullptr, nullptr);
					DoOnce = true;
					return;
				}
			}
			else
			{
				//Reset gate
				DoOnce = false;
			}
		}
	}
}

void AAI_Animal_FOX::DestroyAfterDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Fox ded3"));
	UE_LOG(LogTemp, Warning, TEXT("Fox ded4"));
	this->Destroy();
	GetWorldTimerManager().ClearTimer(DeadTimer);
}
void AAI_Animal_FOX::Dying()
{
	//UHealthComponent* HealthComp = this->FindComponentByClass<UHealthComponent>();
	if (HealthComponent->Health == 0)
	{
		if (DoOnce2 == false)
		{
			DoOnce2 = true;
			bDied = true;
			UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
			if (MoveComp)
			{
				MoveComp->StopActiveMovement();
				MoveComp->DisableMovement();
				UE_LOG(LogTemp, Warning, TEXT("Fox ded2"));

			}
			UE_LOG(LogTemp, Warning, TEXT("Fox ded"));

			GetWorldTimerManager().SetTimer(DeadTimer, this, &AAI_Animal_FOX::DestroyAfterDeath, 2.10f, false);
			//this->Destroy();
		}
	}
}



// Called every frame
void AAI_Animal_FOX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Hearing();

	Attacking();

	Dying();

	//if (IsAttacking == true)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Atakuje"));
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("NIE Atakuje"));
	//}
	AAI_Animal_Controller* AICont = Cast<AAI_Animal_Controller>(this->GetController());
	if (AICont)
	{
		test1 = AICont->IsMoving;
		UE_LOG(LogTemp, Warning, TEXT("Ismovingto: %i"), test1);
		test2 = AICont->IsRunning;
	}
}

// Called to bind functionality to input
void AAI_Animal_FOX::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAI_Animal_FOX::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAI_Animal_FOX, test1);
	DOREPLIFETIME(AAI_Animal_FOX, test2);

}