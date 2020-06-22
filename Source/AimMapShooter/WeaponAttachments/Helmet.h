// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAttachments/BaseAttachmentClass.h"
#include "Helmet.generated.h"


UCLASS()
class AIMMAPSHOOTER_API AHelmet : public ABaseAttachmentClass
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHelmet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	int32 NumberOfHits;

public:	

	int32 GetNumberOfHits();

};
