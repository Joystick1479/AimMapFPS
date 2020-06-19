// Bartosz Jastrzebski


#include "SniperRifle.h"

ASniperRifle::ASniperRifle()
{
	PrimaryActorTick.bCanEverTick = true;
	RateOfFire = 0.0f;
}
void ASniperRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}