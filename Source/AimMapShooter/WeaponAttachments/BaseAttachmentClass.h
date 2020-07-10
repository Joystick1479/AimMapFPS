// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseAttachmentClass.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class AIMMAPSHOOTER_API ABaseAttachmentClass : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseAttachmentClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	bool bPickedUp;

public:	

	bool GetIfPickeditem();
	void SetIfPickedItem(bool PickedUp);
	UStaticMeshComponent* GetMeshComponent();
	USphereComponent* GetSphereComponent();
	

};
