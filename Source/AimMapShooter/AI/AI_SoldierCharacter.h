// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "Character/SoldierCharacter.h"
#include "AI_SoldierCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API AAI_SoldierCharacter : public ASoldierCharacter
{
	GENERATED_BODY()
	
protected:

	virtual void StartFire() override;
	virtual void StopReload() override;
};
