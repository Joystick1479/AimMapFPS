// Bartosz Jastrzebski


#include "AutomaticRifle.h"

#include "Character/SoldierCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"



void AAutomaticRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	CalculateWeaponSway();
}
void AAutomaticRifle::CalculateWeaponSway()
{
	//TODO weapon sway prototype :)
	ASoldierCharacter* SoldierCharOwner = Cast<ASoldierCharacter>(GetOwner());
	if (SoldierCharOwner)
	{
		InitialWeaponSway2 = SoldierCharOwner->GetActorRotation();



		if (InitialWeaponSway2 != FinalWeaponSway2)
		{
			UE_LOG(LogTemp, Warning, TEXT("True"));
			if (InitialWeaponSway2.Yaw > FinalWeaponSway2.Yaw)
			{
				FinalWeaponSway2 = InitialWeaponSway2;

				DirectionSway2 = 10;
				//SetWeaponSway(DirectionSway);
			}
			else
			{
				FinalWeaponSway2 = InitialWeaponSway2;

				DirectionSway2 = (-10);
				//SetWeaponSway(DirectionSway);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("false"));

			FRotator CurrentRotation = GetActorRotation();
			FRotator TargetRotation = FRotator(CurrentRotation.Roll, 0.0f, CurrentRotation.Yaw);
			tescik2 = UKismetMathLibrary::RInterpTo(GetActorRotation(), FRotator(GetActorRotation().Roll, 0.0f, GetActorRotation().Yaw), UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * SmoothSway, 3.0f);
			SetActorRotation(tescik2);


		}
	}

}

void AAutomaticRifle::SetWeaponSway(float SwayDirection)
{
	FRotator CurrentWeaponRotation = GetActorRotation();
	FRotator TargetWeaponRotation = FRotator(GetActorRotation().Roll, GetActorRotation().Pitch + SwayDirection, GetActorRotation().Yaw);
	TargetWeaponRotation.Pitch = UKismetMathLibrary::Clamp(TargetWeaponRotation.Pitch, -5.0f, 5.0f);

	SetActorRotation(UKismetMathLibrary::RInterpTo(CurrentWeaponRotation, TargetWeaponRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld())*SmoothSway, 0.5f));

	UE_LOG(LogTemp, Warning, TEXT("Clamped pitch rotation: %f"), TargetWeaponRotation.Pitch);


}