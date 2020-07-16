// Bartosz Jastrzebski


#include "AI_SoldierCharacter.h"

#include "TimerManager.h"
#include "Weapons/BaseWeaponClass.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h" 


void AAI_SoldierCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetCurrentAmmoInClip() > 0 && CurrentWeapon->CurrentState != EWeaponState::Reloading)
		{
			bFireAnimation = true;
			CurrentWeapon->StartFire();
		}
	}
}

void AAI_SoldierCharacter::StopReload()
{
	bReloading = false;
}