// Bartosz Jastrzebski


#include "FlashGrenade.h"

#include "Components/StaticMeshComponent.h"

#include "Sound/SoundBase.h" 
#include "Sound/SoundAttenuation.h" 

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "SoldierCharacter.h"
#include "GrenadeDecal.h"

#include "TimerManager.h" 

#include "Net/UnrealNetwork.h"


// Sets default values
AFlashGrenade::AFlashGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SetReplicates(true);

}

// Called when the game starts or when spawned
void AFlashGrenade::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		FVector ImpulseVector = MeshComp->GetForwardVector() * ImpulseForce;
		FVector Impulse2Vector = UKismetMathLibrary::RandomUnitVector() * ImpulseForce2;
		ThrowinGrenade(ImpulseVector, Impulse2Vector);
	}
}

void AFlashGrenade::ThrowinGrenade(FVector AddForce, FVector AddForce2)
{
	if (Role < ROLE_Authority)
	{
		ServerThrowinGrenade(AddForce, AddForce2);
	}

	PinPullSound();

	if (MeshComp)
	{
		MeshComp->AddImpulse(Impulse, NAME_None, true);
		MeshComp->AddAngularImpulseInDegrees(Impulse, NAME_None, true);
		GetWorldTimerManager().SetTimer(TimerHandle_Explosion, this, &AFlashGrenade::SpawnEffects, 3.0f);
	}
}

void AFlashGrenade::SpawnEffects()
{
	if (Role < ROLE_Authority)
	{
		ServerSpawnEffects();
	}

	ExplosionSound();

	SpawnExplosionDecal();
}

void AFlashGrenade::SpawnExplosionDecal()
{
	if (Role == ROLE_Authority)
	{
		ServerSpawnExplosionDecal();
	}

	FRotator DecalRotation = FRotator(90, 0, 0);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GrenadeDecal = GetWorld()->SpawnActor<AGrenadeDecal>(GrenadeDecalClass, this->GetActorLocation(), DecalRotation, SpawnParams);

	TArray<AActor*> Character;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierChar, Character);

	for (int i = 0; i < Character.Num(); i++)
	{
		ASoldierCharacter*New = Cast<ASoldierCharacter>(Character[i]);
		if (New)
		{
			float DistanceBetweenActors = FVector::Dist(this->GetActorLocation(), New->GetActorLocation());
			New->Flashbang(DistanceBetweenActors, this->GetActorLocation());
			Destroy();
		}
	}
}

void AFlashGrenade::PinPullSound()
{
	if (Role == ROLE_Authority)
	{
		ServerPinPullSound();
	}

	UGameplayStatics::PlaySoundAtLocation(this, PinSound, this->GetActorLocation(), 2.0f, 1.0f, 0.0f, SoundAttenuation);
}

void AFlashGrenade::ExplosionSound()
{
	if (Role == ROLE_Authority)
	{
		ServerExplosionSound();
	}
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSoundCue, this->GetActorLocation(), 2.0f, 1.0f, 0.0f, SoundAttenuation);
}

void AFlashGrenade::ServerThrowinGrenade_Implementation(FVector ServerImpulseForce, FVector ServerImpulseForce2)
{
	ThrowinGrenade(ServerImpulseForce, ServerImpulseForce2);
}
bool AFlashGrenade::ServerThrowinGrenade_Validate(FVector ServerImpulseForce, FVector ServerImpulseForce2)
{
	return true;
}
void AFlashGrenade::ServerPinPullSound_Implementation()
{
	MulticastPinPullSound();
}
void AFlashGrenade::MulticastPinPullSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(this, PinSound, this->GetActorLocation(), 2.0f, 1.0f, 0.0f, SoundAttenuation);
}
void AFlashGrenade::ServerExplosionSound_Implementation()
{
	MulticastExplosionSound();
}
void AFlashGrenade::MulticastExplosionSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSoundCue, this->GetActorLocation(), 2.0f, 1.0f, 0.0f, SoundAttenuation);
}
void AFlashGrenade::ServerSpawnEffects_Implementation()
{
	MulticastSpawnEffects();
}
void AFlashGrenade::MulticastSpawnEffects_Implementation()
{
	SpawnEffects();
}
void AFlashGrenade::ServerSpawnExplosionDecal_Implementation()
{
	MulticastSpawnExplosionDecal();
}
void AFlashGrenade::MulticastSpawnExplosionDecal_Implementation()
{
	FRotator DecalRotation = FRotator(90, 0, 0);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GrenadeDecal = GetWorld()->SpawnActor<AGrenadeDecal>(GrenadeDecalClass, this->GetActorLocation(), DecalRotation, SpawnParams);

	TArray<AActor*> Character;
	UGameplayStatics::GetAllActorsOfClass(this, SoldierChar, Character);

	for (int i = 0; i < Character.Num(); i++)
	{
		ASoldierCharacter* SoldierCharacter = Cast<ASoldierCharacter>(Character[i]);
		if (SoldierCharacter)
		{
			float DistanceBetweenActors = FVector::Dist(this->GetActorLocation(), SoldierCharacter->GetActorLocation());
			SoldierCharacter->Flashbang(DistanceBetweenActors, this->GetActorLocation());
			Destroy();
		}
	}
}
// Called every frame
void AFlashGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFlashGrenade::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFlashGrenade, Impulse);
	DOREPLIFETIME(AFlashGrenade, Impulse2);

}

