// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI_Animal_FOX.generated.h"

class USphereComponent;
class ASoldierCharacter;

UCLASS()
class AIMMAPSHOOTER_API AAI_Animal_FOX : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAI_Animal_FOX();

	UPROPERTY(BlueprintReadOnly)
	bool IsAttacking;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Query")
	USphereComponent* HearingSphere;

	UPROPERTY(EditDefaultsOnly, Category = "Query")
	TSubclassOf<ASoldierCharacter> SoldierChar;

	void Hearing();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
