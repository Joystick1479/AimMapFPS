// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Payload.generated.h"

UCLASS()
class AIMMAPSHOOTER_API APayload : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APayload();

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

};
