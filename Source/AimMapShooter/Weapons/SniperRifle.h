// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseWeaponClass.h"
#include "SniperRifle.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API ASniperRifle : public ABaseWeaponClass
{
	GENERATED_BODY()

public:
	ASniperRifle();

private:

	virtual void Tick(float DeltaTime) override;


};
