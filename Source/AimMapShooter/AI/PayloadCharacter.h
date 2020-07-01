// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PayloadCharacter.generated.h"

class ASoldierCharacter;

UCLASS()
class AIMMAPSHOOTER_API APayloadCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APayloadCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UStaticMeshComponent* MeshComp2;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UStaticMeshComponent* RootMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UPhysicsConstraintComponent* PhysicsComp;

	UPROPERTY(EditDefaultsOnly, Category = "EndTarget")
	TSubclassOf<ASoldierCharacter> SoldierCharacterClass;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	FTimerHandle BlueTimerHandle;
	FTimerHandle RedTimerHandle;

	UPROPERTY(BlueprintReadOnly)
	FString FindName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	int32 ForceToPush;

	float Acceleration;

	virtual void PayloadMove(float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPayloadMove(float DeltaTime);

	UPROPERTY(VisibleAnywhere, Category = "Sounds")
	class UAudioComponent* AudioComp;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bShouldPush;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bOnePlayerPushing;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bContestedPushing;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector Translation;


};
