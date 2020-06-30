// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAttachments/BaseAttachmentClass.h"
#include "Laser.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class ABaseWeaponClass;

UCLASS()
class AIMMAPSHOOTER_API ALaser : public ABaseAttachmentClass
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaser();
	
protected:

	FName LaserSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	float LengthOfLaser;

	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	double ThickOfLaser;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPointLightComponent* PointLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp2;


public:

	UPointLightComponent* GetPointLightComponent();
	UStaticMeshComponent* GetScalableMeshComponent();

	void StartLaser(ABaseWeaponClass* WeaponClass);
};
