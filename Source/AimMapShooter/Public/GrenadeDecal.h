// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeDecal.generated.h"

UCLASS()
class AIMMAPSHOOTER_API AGrenadeDecal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeDecal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UDecalComponent* Decal;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
