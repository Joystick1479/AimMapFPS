// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RedEndgame.generated.h"

UCLASS()
class AIMMAPSHOOTER_API ARedEndgame : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARedEndgame();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
