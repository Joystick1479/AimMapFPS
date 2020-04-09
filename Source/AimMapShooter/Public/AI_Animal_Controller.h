// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI_Animal_Controller.generated.h"

class AAI_Animal_FOX;
class ATargetPoint;
class ASoldierCharacter;

/**
 * 
 */
UCLASS()
class AIMMAPSHOOTER_API AAI_Animal_Controller : public AAIController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	AAI_Animal_FOX* GetControlledFox() const;

	UPROPERTY(EditDefaultsOnly)
	FVector CorrectingVectorForSoldierHeight;

	UPROPERTY()
	TArray<AActor*> Waypoints;

	UPROPERTY(EditDefaultsOnly, Category = "Soldier")
	TSubclassOf<ASoldierCharacter> Soldier;

	UFUNCTION()
	ATargetPoint* GetRandomWaypoint();

	UFUNCTION()
	void GoToRandomWaypoint();

	FTimerHandle TimerHandle;

	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UPROPERTY(BlueprintReadOnly)
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly)
	bool IsRunning;
	
	int32 index;
	int32 index2;

};
