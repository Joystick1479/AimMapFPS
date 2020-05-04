// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magazine.generated.h"

UCLASS()
class AIMMAPSHOOTER_API AMagazine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagazine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* SphereComp;

	void NotifyActorBeginOverlap(AActor* OtherActor) override;
	void NotifyActorEndOverlap(AActor* OtherActor) override;

	void DestroyOnUse();
	FTimerHandle DestroyTimer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsPickedUp;
};
