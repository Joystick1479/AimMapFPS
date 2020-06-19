// Bartosz Jastrzebski


#include "SniperRifle.h"

ASniperRifle::ASniperRifle()
{

	PrimaryActorTick.bCanEverTick = true;
	RateOfFire = 0.0f;
}
void ASniperRifle::Test()
{
	Fire();
	UseAmmo();
}

void ASniperRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

//	UE_LOG(LogTemp, Warning, TEXT("Camera socket is: %s"), *CameraSocket.ToString());

}