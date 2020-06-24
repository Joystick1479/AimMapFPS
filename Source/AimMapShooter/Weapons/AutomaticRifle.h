// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseWeaponClass.h"
#include "AutomaticRifle.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API AAutomaticRifle : public ABaseWeaponClass
{
	GENERATED_BODY()
public:
	void CalculateWeaponSway();
	void SetWeaponSway(float SwayDirection);
	virtual void Tick(float DeltaTime);
	FRotator InitialWeaponSway2;
	FRotator FinalWeaponSway2;
	FRotator tescik2;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon sway")
	float SmoothSway2;
	float DirectionSway2;

protected:

	
};
