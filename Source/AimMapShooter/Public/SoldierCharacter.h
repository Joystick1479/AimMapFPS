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
class ARifle_3rd;
class UCapsuleComponent;
class UUserWidget;
class USoundCue;
class UAudioComponent;
class APayloadCharacter;
class ARedEndgame;
class ABlueEndgame;
class AFlashGrenade;

class UMinimapComponent;
class UTimelineComponent;


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

	UPROPERTY(VisibleAnywhere, Category = "FPPMesh")
	USkeletalMeshComponent* FPPMesh;

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	USpringArmComponent* SpringArmRender2;

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	class USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(EditAnywhere, Category = "AnimationWeapon")
	TSubclassOf<UAnimInstance> AnimBp;


	ECharacterState::Type CharacterState;

	EHoldingWeapon::Type HoldingWeaponState;

	EHoldingAttachment::Type HoldingAttachmentState;

	EHoloAttachment::Type HoloEquipState;

	EGripAttachment::Type GripEquipState;

	EHeadsetAttachment::Type HeadsetEquipState;
	
	EHelmetAttachment::Type HelmetEquipState;

	ELaserAttachment::Type LaserEquipState;

	UPROPERTY(Replicated)
	AAutomaticRifle* AutomaticRifle;

	UPROPERTY(Replicated)
	ARifle_3rd* Rifle_3rd;

	UPROPERTY(Replicated)
	AHoloScope* HoloScope;

	UPROPERTY(Replicated)
	AGrip* Grip;

	UPROPERTY(Replicated)
	AHeadset* Headset;

	UPROPERTY(Replicated)
	ALaser* Laser;

	UPROPERTY(Replicated)
	AHelmet* Helmet;
	////***Replication stuff***///
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickUpItem();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerZoomIn();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerZoomOut();

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
	void ServerShowingPickUpHud();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnOnLaser();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadingSound();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReloadingSound();

	/////*** DISPLAYING HUD ****////
	void ShowingPickUpHud();

	///**STOP PLAY LOW HEALTH/DIE AUDIO **//
	void DyingAudioTrigger();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLineTraceItem();

	void LineTraceItem();

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
	bool isAllowClimbing;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool isWallThick;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool isAbleToVault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	bool isObjectTooHigh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vault")
	FTimerHandle TimerHandle_Vault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Animation")
	UAnimMontage* ClimbAnim;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Animation")
	UAnimMontage* VaultAnim;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vaulting")
	bool GoVault;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vaulting")
	bool GoClimb;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Vaulting")
	float MaxHeightForVault;

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player")
	TSubclassOf<AAutomaticRifle> StarterWeaponClass;

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

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Weapon")
	float ZoomingTime;



	//Character Movement//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void AddPichInput(float Value);
	void AddYawInput(float Value);

	void BeginCrouch();
	void EndCrouch();

	void StartFire();
	void StopFire();

	void SprintOn();
	void SprintOff();
	void SprintProgressBar();
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Player")
	float stamina;
	FTimerHandle SprintTimerHandle;
	FTimerHandle SprintRestingTimerHandle;

	void FireMode();

	void Reload();
	FTimerHandle ReloadTimer;
	void StopReload();

	void ZoomIn();
	void ZoomOut();

	void PickUp();

	void WeaponInspectionOn();
	void WeaponInspectionOff();

	void PutWeaponOnBack();
	bool bWeaponOnBack;
	FTimerHandle InspectionTimer;

	UPROPERTY(BlueprintReadOnly)
	bool IsInspecting;

	UPROPERTY(EditDefaultsOnly, Category = "LineTrace")
	float MaxUseDistance;

	///HEALTH AND SURVIVAL COMPONENTS///

	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USurvivalComponent* SurvivalComp;

	void EatFood();

	void DrinkWater();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEatFood();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDrinkWater();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPutWeaponOnBack();



	void EndDrinkFromPond(APlayerController* PC);


	/////*SOUNDS WHEN PICK UP OBJECTS***////

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

	bool bStopSound;

	UPROPERTY(EditDefaultsOnly, Category = "FlashMaterial")
	UMaterialParameterCollection* MaterialCollection;

	UPROPERTY(EditDefaultsOnly, Category = "Payload")
	TSubclassOf<APayloadCharacter> PayloadCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "EndTarget")
	TSubclassOf<ARedEndgame> ClientEndgameClass;

	UPROPERTY(EditDefaultsOnly, Category = "EndTarget")
	TSubclassOf<ABlueEndgame> HostEndgameClass;

	void DefendObjectiveSound();

	void GameOverSound();


	void NotifyActorBeginOverlap(AActor * OtherActor);
	
	class UStaticMeshComponent* MeshComp;

	UFUNCTION(BlueprintCallable,Server,Reliable)
	void ServerWantToRespawn();


public:	

	
	///GRENADE///
	void ThrowGrenade();
	void SpawnGrenade(FVector STL, FRotator STR);
	void FindingGrenadeTransform();
	void Flashbang(float Distance, FVector FacingAngle);
	void AngleFromFlash(FVector GrenadeLoc);
	///Timeline for GRENADE//
	void FlashTimeline();
	UFUNCTION()
	void TimelineCallback(float val);
	UFUNCTION()
	void TimelineFinishedCallback();
	void PlayTimeline();

	bool IsFacing;
	float FlashAmount;
	//UPROPERTY(Replicated)
	float Distance;
	//UPROPERTY(Replicated)
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

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

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
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated,Category = "Zoom")
	bool IsZooming;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Sprint")
	bool IsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Zoom")
	bool IsSingleFire;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Replicated, Category = "Zoom")
	bool IsFiring;
	///Bool for crouching///
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Crouch")
	bool IsCrouching;


	UPROPERTY(BlueprintReadOnly)
	bool bFireAnimation;

	///Bool for BackTargeting ///
	void IsTargetFromBack();
	bool MultipleDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASoldierCharacter> SoldierChar;

	///Bool for reloading///
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Reloading")
	bool IsReloading;

	//* Bool if we can pickup Rifle *//
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player")
	bool bRiflePickUp;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool bHoloPickUp;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool bGripPickUp;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool bHelmetPickUp;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool bHeadsetPickUp;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool bLaserPickUp;



	UPROPERTY(EditDefaultsOnly, Replicated,Category = "Player")
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


	UPROPERTY(BlueprintReadOnly, Replicated,Category = "Player")
	bool bRemoveHud;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool isWeaponAttached;

	//Weapon Sway
	FRotator InitialWeaponRot;
	UPROPERTY(BlueprintReadOnly)
	FRotator FinalWeaponRot;
	UPROPERTY(EditDefaultsOnly)
	float SmoothAmount;
	UPROPERTY(EditDefaultsOnly)
	float LookAmount;
	FName LookUp;
	UPROPERTY(BlueprintReadWrite)
	float temp1;
	UPROPERTY(BlueprintReadWrite)
	float temp2;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isLaserAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isHeadsetAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isGripAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isHoloAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isHelmetAttached;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool isLaserON;

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

	void OnFoodLow();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnDrinkLow();

	void OnDrinkLow();

	FTimerHandle FoodLowTimer;
	FTimerHandle DrinkLowTimer;
	FTimerHandle DrinkFromPondTimer;

	bool bDrinkPickup;
	UPROPERTY(BlueprintReadOnly)
	float amountOfDrinks;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	TSubclassOf<class ADrink> DrinkClass;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	TSubclassOf<class AFood> FoodClass;

	bool bFoodPickup;

	bool bDrinkFromPond;

	UPROPERTY(BlueprintReadOnly)
	float amountOfFood;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	float amountOfBoostFood;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	float amountOfBoostDrink;

	UPROPERTY(EditDefaultsOnly,Category = "Survival items")
	USoundCue* EatFoodSound;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	USoundCue* DrinkWaterSound;

	UPROPERTY(EditDefaultsOnly, Category = "Survival items")
	USoundCue* DrinkFromPondSound;

	UPROPERTY(Replicated)
	bool bWantsToRepawn;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
