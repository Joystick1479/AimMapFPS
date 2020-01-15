// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlueEndgame.generated.h"

UCLASS()
class AIMMAPSHOOTER_API ABlueEndgame : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlueEndgame();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* SphereComp;

public:	
	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor* OtherActor);

	UPROPERTY(BlueprintReadOnly)
	bool BlueWins;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
