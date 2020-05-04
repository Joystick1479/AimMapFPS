// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include <stack>

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

	virtual void Tick(float DeltaTime) override;

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

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_GoToRandomWaypoint();

	FTimerHandle TimerHandle;
	FTimerHandle TestTimerHandle;

	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly,Replicated)
	bool IsRunning;
	
	//Don't take the same number twice in a row.
	int32 index;
	int32 index2;
	std::stack<int> dupa;

	FTimerHandle FastAttackTimer;

	


protected:

	//For trigger animation
	UPROPERTY(BlueprintReadOnly)
	bool bAttacking;

	void CheckIfChargePlayer();
	FTimerHandle ChargeTimer;

private:

	int32 randomNumber;
	bool DoOnce;
	

	void FastAttack(AAI_Animal_FOX* Fox);


};
