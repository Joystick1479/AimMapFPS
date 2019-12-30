// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AimMapGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API UAimMapGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UAimMapGameInstance(const FObjectInitializer & ObjectInitializer);

	virtual void Init();
	
};
