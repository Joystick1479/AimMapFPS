// Bartosz Jastrzebski

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AIMMAPSHOOTER_API USurvivalComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USurvivalComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "SurvivalComponent")
	float DefaultFood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "SurvivalComponent")
	float DefaultDrink;


	UFUNCTION()
	void OnRep_Food();

	UFUNCTION()
	void OnRep_Drink();

	UPROPERTY(EditDefaultsOnly,Category = "Food")
	float FreQOfDrainingFood;

	UPROPERTY(EditDefaultsOnly, Category = "Drink")
	float FreQOfDrainingDrink;

	FTimerHandle FoodTimerHandle;
	FTimerHandle DrinkTimerHandle;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(ReplicatedUsing = OnRep_Food, EditAnywhere, BlueprintReadWrite, Category = "SurvivalComponent")
	float Food;

	UPROPERTY(ReplicatedUsing = OnRep_Drink, EditAnywhere, BlueprintReadWrite, Category = "SurvivalComponent")
	float Drink;
};
