// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI_Animal_FOX.generated.h"

class USphereComponent;
class ASoldierCharacter;
class UHealthComponent;
class UCapsuleComponent;

UCLASS()
class AIMMAPSHOOTER_API AAI_Animal_FOX : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAI_Animal_FOX();

	UPROPERTY(BlueprintReadOnly)
	bool IsAttacking;

	UPROPERTY(BlueprintReadOnly)
	bool bDied;

	UCharacterMovementComponent* GetCharacterMovementComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Query")
	USphereComponent* HearingSphere;

	UPROPERTY(VisibleAnywhere, Category = "Query")
	UCapsuleComponent* DamagingSphere;

	UPROPERTY(VisibleAnywhere, Category = "Query")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Query")
	TSubclassOf<ASoldierCharacter> SoldierChar;

	void Hearing();
	FTimerHandle HearingHandle;

	void Attacking();
	FTimerHandle AttackingHandle;

	void Dying();
	FTimerHandle DyingHandle;

	void DestroyAfterDeath();

	void UpdateFStatus();
	FTimerHandle UpdateHandle;



	FTimerHandle DeadTimer;

	bool DoOnce;

	bool DoOnce2;

	UPROPERTY(BlueprintReadOnly,Replicated)
	bool test1;

	UPROPERTY(BlueprintReadOnly,Replicated)
	bool test2;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
