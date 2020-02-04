// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlashGrenade.generated.h"

class ASoldierCharacter;

UCLASS()
class AIMMAPSHOOTER_API AFlashGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlashGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Pawn")
	TSubclassOf<ASoldierCharacter> SoldierChar;

	void ThrowinGrenade(FVector Impulse, FVector Impulse2);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
