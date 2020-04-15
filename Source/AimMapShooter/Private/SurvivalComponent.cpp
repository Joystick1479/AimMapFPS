// Bartosz Jastrzebski


#include "SurvivalComponent.h"

#include "Net/UnrealNetwork.h"

#include "TimerManager.h"

#include "SoldierCharacter.h"



// Sets default values for this component's properties
USurvivalComponent::USurvivalComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	DefaultDrink = 100;
	DefaultFood = 100;
	FreQOfDrainingDrink = 5.0f;
	FreQOfDrainingFood = 10.0f;
	// ...
}


// Called when the game starts
void USurvivalComponent::BeginPlay()
{
	Super::BeginPlay();

	Food = DefaultFood;
	Drink = DefaultDrink;
	// ...
	OnRep_Food();
	OnRep_Drink();
}


void USurvivalComponent::OnRep_Food()
{
	if (Food > 0)
	{
		Food = Food - 1;

		UE_LOG(LogTemp, Warning, TEXT("Food is: %f"), Food);
		GetWorld()->GetTimerManager().SetTimer(FoodTimerHandle, this, &USurvivalComponent::OnRep_Food, FreQOfDrainingFood, false);
	}


	if (Food == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(FoodTimerHandle);

		ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
		{
			if (SoldierChar)
			{
				SoldierChar->OnFoodLow();
			}
		}
	}
	
}

void USurvivalComponent::OnRep_Drink()
{
	if (Drink > 0)
	{
		Drink = Drink - 1;

		UE_LOG(LogTemp, Warning, TEXT("Drink is: %f"), Drink);
		GetWorld()->GetTimerManager().SetTimer(DrinkTimerHandle, this, &USurvivalComponent::OnRep_Drink, FreQOfDrainingDrink, false);
	}
	if (Drink == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(DrinkTimerHandle);

		ASoldierCharacter* SoldierChar = Cast<ASoldierCharacter>(GetOwner());
		{
			if (SoldierChar)
			{
				SoldierChar->OnDrinkLow();
			}
		}
	}


}

// Called every frame
void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USurvivalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//This function tells us how we want to replicate things//
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USurvivalComponent, Drink);
	DOREPLIFETIME(USurvivalComponent, DefaultDrink);
	DOREPLIFETIME(USurvivalComponent, Food);
	DOREPLIFETIME(USurvivalComponent, DefaultFood);
}