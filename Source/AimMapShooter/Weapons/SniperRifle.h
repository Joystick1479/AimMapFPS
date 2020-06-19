// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "SniperRifle.h"
#include "SniperRifle.generated.h"

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API ASniperRifle : public AAutomaticRifle
{
	GENERATED_BODY()

public:
	ASniperRifle();

private:

	virtual void Tick(float DeltaTime) override;


};
