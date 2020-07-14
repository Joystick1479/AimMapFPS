// Bartosz Jastrzebski


#include "AI_SoldierCharacter.h"

#include "TimerManager.h"
#include "Weapons/BaseWeaponClass.h"

void AAI_SoldierCharacter::StartFire()
{
	
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetCurrentAmmoInClip() > 0 && CurrentWeapon->CurrentState != EWeaponState::Reloading)
		{
			bFireAnimation = true;
			CurrentWeapon->StartFire();
		}
		if (CurrentWeapon->GetCurrentAmmoInClip() == 0)
		{
			bReloading = true;
			CurrentWeapon->StartReload();
			GetWorldTimerManager().SetTimer(ReloadTimer, this, &AAI_SoldierCharacter::StopReload, 2.167f, false);
		}
	}
	
}

void AAI_SoldierCharacter::StopReload()
{
	bReloading = false;
}