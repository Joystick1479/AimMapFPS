// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PayloadCharacter.generated.h"

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
