// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAttachments/BaseAttachmentClass.h"
#include "HoloScope.generated.h"


UCLASS()
class AIMMAPSHOOTER_API AHoloScope : public ABaseAttachmentClass
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHoloScope();

	virtual FName GetAttachPoint() const override;

};
