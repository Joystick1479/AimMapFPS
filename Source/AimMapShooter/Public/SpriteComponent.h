// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteComponent.h"
#include "SpriteComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AIMMAPSHOOTER_API USpriteComponent : public UPaperSpriteComponent
{
	GENERATED_BODY()
	

public:
	// Sets default values for this component's properties
	USpriteComponent();
};
