// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseAttachmentClass.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class ASoldierCharacter;

UCLASS()
class AIMMAPSHOOTER_API ABaseAttachmentClass : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseAttachmentClass();
	~ABaseAttachmentClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh3P;

	ASoldierCharacter* PawnOwner = nullptr;

	virtual FName GetAttachPoint() const;

	bool bAttachmentAttached = false;

	bool bPickedUp = false;

public:	

	UFUNCTION(BlueprintPure)
	virtual bool bIsAttachmentAttached()const;


	virtual void PickUpAttachment();
	bool GetIfPickeditem();
	void SetOwningPawn(ASoldierCharacter* SoldierCharacter);
	void SetIfPickedItem(bool PickedUp);
	UStaticMeshComponent* GetMesh1P();
	UStaticMeshComponent* GetMesh3P();
	USphereComponent* GetSphereComponent();
	

};
