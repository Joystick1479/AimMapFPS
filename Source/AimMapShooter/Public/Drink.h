// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Drink.generated.h"

UCLASS()
class AIMMAPSHOOTER_API ADrink : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADrink();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* SphereComp;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);

	void DestroyOnUse();
	FTimerHandle DestroyTimer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsPickedUp;

};
