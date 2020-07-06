// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "SoldierCharacter.generated.h"

class UCameraComponent;
class AAutomaticRifle;
class USkeletalMeshComponent;
class UAnimInstance;
class UHealthComponent;
class USurvivalComponent;
class AHoloScope;
class AGrip;
class USpringArmComponent;
class AHelmet;
class AHeadset;
class ALaser;
class AMagazine;
class ARifle_3rd;
class UCapsuleComponent;
class UUserWidget;
class USoundCue;
class UAudioComponent;
class APayloadCharacter;
class AFlashGrenade;
class UMinimapComponent;
class UTimelineComponent;
class ASniperRifle;
class ABaseWeaponClass;
class ABaseAttachmentClass;
class ABaseSurvivalItemClass;


namespace ECharacterState
{
	enum Type
	{
		Idle,
		Walking,
		Firing,
		Reloading
	};
}

namespace EHoldingWeapon
{
	enum Type
	{
		None,
		A4,
		Sniper,
	};
}

namespace EHoldingAttachment
{
	enum Type
	{
		None,
		Holo,
		Grip,
	};
}
namespace EHoloAttachment
{
	enum Type
	{
		None,
		Equipped,
	};
}
namespace EGripAttachment
{
	enum Type
	{
		None,
		Equipped,
	};
}
namespace EHeadsetAttachment
{
	enum Type
	{
		None,
		Equipped,
	};
}
namespace ELaserAttachment
{
	enum Type
	{
		None,
		Equipped,
	};
}
namespace EHelmetAttachment
{
	enum Type
	{
		None,
		Equipped,
	};
}

UCLASS()
class AIMMAPSHOOTER_API ASoldierCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASoldierCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	ECharacterState::Type CharacterState;

	EHoldingWeapon::Type HoldingWeaponState;

	EHoldingAttachment::Type HoldingAttachmentState;

	EHoloAttachment::Type HoloEquipState;

	EGripAttachment::Type GripEquipState;

	EHeadsetAttachment::Type HeadsetEquipState;

	ELaserAttachment::Type LaserEquipState;

	EHelmetAttachment::Type HelmetEquipState;


	//*Vaulting stuff//*
	void Vault();
	void ResetVaultTimer();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector WallLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector WallNormal;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector WallHight;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FVector NextWallHight;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool bIsAllowClimbing;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool bIsWallThick;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool bIsAbleToVault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool bIsObjectTooHigh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FTimerHandle TimerHandle_Vault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Animation")
	UAnimMontage* ClimbAnim;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Animation")
	UAnimMontage* VaultAnim;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vaulting")
	bool bGoVault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vaulting")
	bool bGoClimb;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Vaulting")
	float MaxHeightForVault;

	///Components/Actor classess
	UPROPERTY(VisibleAnywhere, Category = "FPPMesh")
	USkeletalMeshComponent* FPPMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USurvivalComponent* SurvivalComp;

	UPROPERTY(EditAnywhere, Category = "AnimationWeapon")
	TSubclassOf<UAnimInstance> AnimBp;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<AAutomaticRifle> AutoRifleClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<ASniperRifle> SniperRifleClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<ARifle_3rd> ThirdWeaponClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<AHoloScope> HoloClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<AGrip> GripClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<AHelmet> HelmetClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<AHeadset> HeadsetClass;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<ALaser> LaserClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AMagazine> MagazineClass;

	UPROPERTY(EditDefaultsOnly, Category = "Payload")
	TSubclassOf<APayloadCharacter> PayloadCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UCameraShake> CameraSprintShake;

	ARifle_3rd* Rifle_3rd;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Weapon")
	float ZoomingTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	float stamina;

	bool bResetBreath;

	bool bWeaponOnBack;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInspecting;

	UPROPERTY(EditDefaultsOnly, Category = "LineTrace")
	float MaxUseDistance;

	bool bDoRagdollOnce;

	bool bStopSound;


	//Character Movement//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void AddPichInput(float Value);
	void AddYawInput(float Value);
	void BeginCrouch();
	void EndCrouch();

	UFUNCTION(BlueprintCallable)
	void StartFire();

	void StopFire();
	void SprintOn();
	void SprintOff();
	void SprintProgressBar();
	void SprintSlowDown();
	void OutOfBreathSound();
	void OutOfBreathReset();
	void Headbobbing();
	void UpdateRifleStatus();
	void FireMode();
	void Reload();
	void StopReload();
	void ZoomIn();
	void ZoomOut();

	void PickUp(ABaseWeaponClass* Weapons,ABaseAttachmentClass* Attachments, ABaseSurvivalItemClass* SurvivalItem);
	bool bWantToPickUp;

	void WeaponInspectionOn();
	void WeaponInspectionOff();
	void PutWeaponOnBack();
	void RagdollOnDeath();
	void DrinkWater();
	void EatFood();
	void EndDrinkFromPond(APlayerController* PC);
	void GrenadeTimeline();
	void ShowingPickUpHud();
	void LineTraceItem();

	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/////*SOUNDS***////

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* RiflePickUp;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* ItemsPickUp;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* LowHealthSound;

	UPROPERTY(VisibleAnywhere, Category = "Sounds")
	UAudioComponent* AudioComp;

	UPROPERTY(VisibleAnywhere, Category = "Sounds")
	UAudioComponent* AudioCompReload;

	UPROPERTY(VisibleAnywhere, Category = "Sounds")
	UAudioComponent* AudioDamageComp;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* EscortVehicle;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* DefendObjective;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* GameOverAudio;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* FiremodeSwitch;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* OutOfBreath;

	UPROPERTY(EditDefaultsOnly, Category = "FlashMaterial")
	UMaterialParameterCollection* MaterialCollection;




	///Multiplayer Replication
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerWantToRespawn();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEatFood();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDrinkWater();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPutWeaponOnBack();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickUpItem(ABaseWeaponClass* Weapons, ABaseAttachmentClass* Attachments, ABaseSurvivalItemClass* SurvivalItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSprintOn();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSprintOff();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerVault();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerResetTimerVault();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginCrouch();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndCrouch();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnOnLaser();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadingSound();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReloadingSound();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLineTraceItem();


	UPROPERTY(Replicated, BlueprintReadOnly)
	ASniperRifle* SniperRifle;

	UPROPERTY(Replicated, BlueprintReadOnly)
	AAutomaticRifle* AutomaticRifle;

	AHoloScope* HoloScope;

	AGrip* Grip;

	AHeadset* Headset;

	AMagazine* Magazine;

	ALaser* Laser;

	AHelmet* Helmet;

	UPROPERTY(Replicated)
	ABaseWeaponClass* CurrentWeapon;

	///Timers
	FTimerHandle TimelineHandle;
	FTimerHandle HudTimer;
	FTimerHandle UpdateWRotTimer;
	FTimerHandle RagdollTimer;
	FTimerHandle InspectionTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle UpdateRifleTimer;
	FTimerHandle SprintRestingTimerHandle;
	FTimerHandle SprintTimerHandle;
	FTimerHandle HBobingTimer;
	FTimerHandle UpdateBreath;
	FTimerHandle OutOfBreathTimer;
	FTimerHandle SlowDownSprintTimer;
	FTimerHandle UpdateSurvivalComponent;

	///GRENADE///
	void ThrowGrenade();
	void SpawnGrenade(FVector StartingLocation, FRotator StartingRotation);
	void FindingGrenadeTransform();
	FTimerHandle TransformHandle;
	void AngleFromFlash(FVector GrenadeLoc);
	///Timeline for GRENADE//
	UFUNCTION()
	void TimelineCallback(float val);
	UFUNCTION()
	void TimelineFinishedCallback();
	void PlayTimeline();
	bool bIsFacing;
	float FlashAmount;
	float Distance;
	FVector FacingAngle;
	UPROPERTY(Replicated)
	FVector STL;
	UPROPERTY(Replicated)
	FRotator STR;
	UPROPERTY(EditDefaultsOnly)
	int32 AmountGrenades;
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* MyTimeline;
	UPROPERTY()
	UCurveFloat* FloatCurve;

	UPROPERTY()
	TEnumAsByte<ETimelineDirection::Type> TimelineDirection;

	FTimerHandle Timer_Flash;

	UPROPERTY(Replicated)
	AFlashGrenade* FlashGrenade;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	TSubclassOf<AFlashGrenade> FlashGrenadeClass;

	///GRENADE REPLICATION///
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnGrenade();

	UFUNCTION(Server, Reliable)
	void ServerFlashbang(FVector Facing);

	UFUNCTION(Client, Reliable)
	void MulticastFlashbang(FVector Facing);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade")
	class USceneComponent* GrenadeStartLocation;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FString PlayerName;

	/// Variable for sprinting ///
	float MaxWalkSpeed;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Camera")
	UCameraComponent* CameraComp;

	void TurnOnLaser();

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		USpringArmComponent* SpringArm;

	FName HeadSocket;
	FName ArmSocket;
	FName WeaponSocket;
	FName HelmetSocket;
	FName HeadsetSocket;
	FName WeaponBackSocket;



	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Sprint")
	bool bIsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Zoom")
	bool bIsSingleFire;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Zoom")
	bool bIsFiring;
	///Bool for crouching///
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Crouch")
	bool bIsCrouching;



	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASoldierCharacter> SoldierChar;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<UUserWidget> wPickUp;

	UPROPERTY(Replicated)
	UUserWidget* wPickUpvar;


	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<UUserWidget> wAmmoCount;

	UPROPERTY(Replicated)
	UUserWidget* wAmmoCountvar;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<UUserWidget> wHealthIndicator;

	UPROPERTY(EditDefaultsOnly, Category = "widget")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(Replicated)
	UUserWidget* wHealthIndicatorvar;

	UPROPERTY(EditDefaultsOnly, Category = "widget")
	TSubclassOf<UUserWidget> PickUpTestWidgetClass;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool bRemoveHud;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player")
	bool bIsWeaponAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsLaserAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsHeadsetAttached;


	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsHoloAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsHelmetAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 SoldierCurrentAmmoInClip;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 SoldierCurrentAmmo;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 SoldierCurrentClips;


	///SURVIVAL///
	UPROPERTY(EditDefaultsOnly, Category = "Survival")
	float FreQOfDrainingHealthWhenLowFood;

	UPROPERTY(EditDefaultsOnly, Category = "Survival")
	float FreQOfDrainingHealthWhenLowDrink;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnFoodLow();


	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnDrinkLow();

	FTimerHandle FoodLowTimer;
	FTimerHandle DrinkLowTimer;
	FTimerHandle DrinkFromPondTimer;

	UPROPERTY(BlueprintReadOnly)
	float amountOfDrinks;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	TSubclassOf<class ADrink> DrinkClass;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	TSubclassOf<class AFood> FoodClass;



	UPROPERTY(BlueprintReadOnly)
	float amountOfFood;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	float amountOfBoostFood;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	float amountOfBoostDrink;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	USoundCue* EatFoodSound;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	USoundCue* DrinkWaterSound;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	USoundCue* DrinkFromPondSound;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	//Implementing Multithreading test
	UFUNCTION(BlueprintCallable, Category = "Multithreading")
	void CalculatePrimeNumbers();

	UFUNCTION(BlueprintCallable, Category = "Multithreading")
	void CalculatePrimeNumbersAsync();

	UPROPERTY(EditAnywhere, Category = MultiThreading)
	int32 MaxPrime;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	//Zooming stuff
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Zoom")
	bool bZooming;
	float AimAlpha;
	UPROPERTY(EditDefaultsOnly, Category = "Zoom")
	float ZoomInterpSpeed;
	float FieldOfView;
	UPROPERTY(EditDefaultsOnly, Category = "Zoom")
	float HoloScopeFieldOfView;
	UPROPERTY(EditDefaultsOnly, Category = "Zoom")
	float NoScopeFieldOfView;


	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bGripAttached;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Reloading")
	bool bReloading;

	UPROPERTY(Replicated)
	bool bWantsToRepawn;

	bool bDoOnce = true;

	void OnFoodLow();
	void OnDrinkLow();
	

public:	

	
	bool GetbDied();
	bool GetbZooming();
	bool GetbGripAttached();
	bool GetbWantToRespawn();

	UPROPERTY(BlueprintReadOnly)
	bool bFireAnimation;
	
	void Flashbang(float ThrowDistance, FVector PlayerFacingAngle);

	
	
};









namespace ThreadingTest
{
	static void CalculatePrimeNumbers(int32 UpperLimit)
	{
		for (int32 i = 1; i <= UpperLimit; i++)
		{
			bool isPrime = true;
			for (int32 j = 2; j <= i / 2; j++)
			{
				if (FMath::Fmod(i, j) == 0)
				{
					isPrime = false;
					break;
				}
			}
			if (isPrime) GLog->Log("Prime number #" + FString::FromInt(i) + ": " + FString::FromInt(i));
		}
	}
}

class PrimeCalculationAsyncTask : public FNonAbandonableTask
{
	int32 MaxPrime;

public:
	PrimeCalculationAsyncTask(int32 MaxPrime)
	{
		this->MaxPrime = MaxPrime;
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(PrimeCalculationAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	void DoWork()
	{
		ThreadingTest::CalculatePrimeNumbers(MaxPrime);
		GLog->Log("--------------------------------------------------------------------");
		GLog->Log("End of prime numbers calculation on background thread");
		GLog->Log("--------------------------------------------------------------------");
	}
};
