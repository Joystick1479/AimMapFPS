// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rifle_3rd.generated.h"

UCLASS()
class AIMMAPSHOOTER_API ARifle_3rd : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARifle_3rd();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* SkelMeshComp;
public:	

	USkeletalMeshComponent* GetSkelMeshComp();

};
