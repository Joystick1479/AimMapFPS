// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerStartActor.generated.h"

class USphereComponent;
/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API APlayerStartActor : public APlayerStart
{
	GENERATED_BODY()

public:

	APlayerStartActor(const FObjectInitializer& ObjectInitializer);
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	bool GetbIsOverlapping();

private:

	bool bIsOverlapping;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp = nullptr;
};
