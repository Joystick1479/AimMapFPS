// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlashGrenade.generated.h"

class ASoldierCharacter;
class AGrenadeDecal;

UCLASS()
class AIMMAPSHOOTER_API AFlashGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlashGrenade();


	UPROPERTY(Replicated)
	FVector Impulse;

	UPROPERTY(Replicated)
	FVector Impulse2;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;





private:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	void ThrowinGrenade(FVector Impulse, FVector Impulse2);

	void SpawnExplosionDecal();

	void PinPullSound();

	void ExplosionSound();

	///MULTIPLAYER REPLICATION///
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerThrowinGrenade();

	UFUNCTION(Server, Reliable)
	void ServerPinPullSound();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPinPullSound();

	UFUNCTION(Server, Reliable)
	void ServerExplosionSound();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastExplosionSound();

	

	UPROPERTY(EditDefaultsOnly, Category = "Pawn")
	TSubclassOf<ASoldierCharacter> SoldierChar;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	TSubclassOf<AGrenadeDecal> GrenadeDecalClass;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	class USoundBase* PinSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	class USoundBase* ExplosionSoundCue;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	class USoundAttenuation* SoundAttenuation;


	UPROPERTY()
	AGrenadeDecal* GrenadeDecal;


	FTimerHandle TimerHandle_Explosion;

public:	


	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
