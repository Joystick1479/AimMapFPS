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

	//TArray<AActor*> Character;
	//UGameplayStatics::GetAllActorsOfClass(this, SoldierChar, Character);

	//for (int i = 0; i < Character.Num(); i++)
	//{
	//	ASoldierCharacter*New = Cast<ASoldierCharacter>(Character[i]);
	//	if (New)
	//	{
			FVector Impulse = MeshComp->GetForwardVector() * 750;
			FVector Impulse2 = UKismetMathLibrary::RandomUnitVector() * 500;
			ThrowinGrenade(Impulse,Impulse2);
	//	}
	//}
	//
}

void AFlashGrenade::ThrowinGrenade(FVector Impulse, FVector Impulse2)
{
	if (Role < ROLE_Authority)
	{
		ServerThrowinGrenade(Impulse, Impulse2);
	}

	PinPullSound();

	MeshComp->AddImpulse(Impulse, NAME_None, true);
	MeshComp->AddAngularImpulseInDegrees(Impulse, NAME_None, true);

	GetWorldTimerManager().SetTimer(TimerHandle_Explosion, this, &AFlashGrenade::SpawnExplosionDecal, 3.0f);

	
}

void AFlashGrenade::SpawnExplosionDecal()
{
	ExplosionSound();

	FRotator DecalRotation = FRotator(0, 90, 0);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GrenadeDecal = GetWorld()->SpawnActor<AGrenadeDecal>(GrenadeDecalClass, this->GetActorLocation(), DecalRotation, SpawnParams);

	GetWorldTimerManager().ClearTimer(TimerHandle_Explosion);
}

void AFlashGrenade::PinPullSound()
{
	if (Role < ROLE_Authority)
	{
		ServerPinPullSound();
	}
	UGameplayStatics::PlaySoundAtLocation(this, PinSound, this->GetActorLocation(), 2.0f, 1.0f, 0.0f, SoundAttenuation);
}

void AFlashGrenade::ExplosionSound()
{
	if (Role < ROLE_Authority)
	{
		ServerExplosionSound();
	}
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSoundCue, this->GetActorLocation(), 2.0f, 1.0f, 0.0f, SoundAttenuation);
}

void AFlashGrenade::ServerThrowinGrenade_Implementation(FVector Impulse, FVector Impulse2)
{
	ThrowinGrenade(Impulse, Impulse2);
}
bool AFlashGrenade::ServerThrowinGrenade_Validate(FVector Impulse, FVector Impulse2)
{
	return true;
}
void AFlashGrenade::ServerPinPullSound_Implementation()
{
	MulticastPinPullSound();
}
void AFlashGrenade::MulticastPinPullSound_Implementation()
{
	PinPullSound();
}
void AFlashGrenade::ServerExplosionSound_Implementation()
{
	MulticastExplosionSound();
}
void AFlashGrenade::MulticastExplosionSound_Implementation()
{
	ExplosionSound();
}

// Called every frame
void AFlashGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

