// Bartosz Jastrzebski


#include "AI_Animal_FOX.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Character/HealthComponent.h"
#include "Character/SoldierCharacter.h"

#include "AI/AI_Animal_Controller.h"

#include "TimerManager.h"

#include "Net/UnrealNetwork.h"

#include "GameFramework/CharacterMovementComponent.h" 


// Sets default values
AAI_Animal_FOX::AAI_Animal_FOX()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	HearingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HearingSphere"));
	HearingSphere->SetupAttachment(this->GetMesh());

	DamagingSphere = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DamagingSphere"));
	DamagingSphere->SetupAttachment(this->GetMesh());

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	SetReplicates(true);
}

UCharacterMovementComponent * AAI_Animal_FOX::GetCharacterMovementComponent()
{
	UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		return MoveComp;
	}
	
	return nullptr;
}

// Called when the game starts or when spawned
void AAI_Animal_FOX::BeginPlay()
{
	Super::BeginPlay();

	Hearing();

	Attacking();

	Dying();

	UpdateFStatus();
	
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
			if (SoldChar->GetbDied() == true)
			{
				IsAttacking = false;
			}
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
	}


	GetWorldTimerManager().SetTimer(HearingHandle, this, &AAI_Animal_FOX::Hearing, 1.0f, false);
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
			if (DamagingSphere->IsOverlappingActor(SoldChar) == true && SoldChar->GetbDied() == false)
			{
				{
					UGameplayStatics::ApplyDamage(SoldChar, 30.0f, nullptr, nullptr, nullptr);
				}
			}
		}
	}

	GetWorldTimerManager().SetTimer(AttackingHandle, this, &AAI_Animal_FOX::Attacking, 0.45f, false);

}

void AAI_Animal_FOX::DestroyAfterDeath()
{
	this->Destroy();
	GetWorldTimerManager().ClearTimer(DeadTimer);
}
void AAI_Animal_FOX::UpdateFStatus()
{
	AAI_Animal_Controller* AICont = Cast<AAI_Animal_Controller>(this->GetController());
	if (AICont)
	{
		test1 = AICont->IsMoving;
		test2 = AICont->IsRunning;
	}

	GetWorldTimerManager().SetTimer(UpdateHandle, this, &AAI_Animal_FOX::UpdateFStatus, 0.35f, false);
}
void AAI_Animal_FOX::Dying()
{
	if (HealthComponent->Health == 0)
	{
		//Clearing attacking timer from AIController to kill the delay
		AAI_Animal_Controller* AIController = Cast<AAI_Animal_Controller>(GetController());
		if (AIController)
		{
			FTimerHandle test = AIController->FastAttackTimer;
			GetWorldTimerManager().ClearTimer(test);
		}
		if (DoOnce2 == false)
		{
			DoOnce2 = true;
			bDied = true;
			UCharacterMovementComponent* MoveComp = this->FindComponentByClass<UCharacterMovementComponent>();
			if (MoveComp)
			{
				MoveComp->StopActiveMovement();
				MoveComp->DisableMovement();
			}
			GetWorldTimerManager().SetTimer(DeadTimer, this, &AAI_Animal_FOX::DestroyAfterDeath, 2.10f, false);
		}
	}
	GetWorldTimerManager().SetTimer(DyingHandle, this, &AAI_Animal_FOX::Dying, 0.35f, false);
}



// Called every frame
void AAI_Animal_FOX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
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