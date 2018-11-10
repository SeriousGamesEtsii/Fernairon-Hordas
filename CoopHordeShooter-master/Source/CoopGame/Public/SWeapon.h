// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SUsableActor.h"
#include "SWeapon.generated.h"

//Contains information of a single hitscan weapon linetrace
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:


	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public ASUsableActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USActorWidgetComponent* ActorWidgetComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCapsuleComponent* CapComp;

	//Dispersión de balas en grados
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (ClampMin = 0.0f))
	float BulletSpread;

	//Animaciones
	void PlayFireEffect(FVector TracerEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* TracerEffect;

	//Daño && Disparos
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName MuzzleSocketName;

	/*Eyección de balas al disparar*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName ShellEjectSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName ClipSocketName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ShellEject")
	TSubclassOf<AActor> BulletClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ClipEject")
	TSubclassOf<AActor> ClipClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Weapon)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	int32 Ammunition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	int32 TotalAmmo;

	bool bCanReload;

	float LastFireTime;

	virtual void Fire();

	//RPM - Bullets per minute fired
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float RateOfFire;

	//Derived from rate of fire
	float TimeBetweenShots;

	/*Ranges*/
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float MaxRangeYaw;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float MinRangeYaw;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float MaxRangePitch;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float MinRangePitch;

	float RecoilYaw;
	
	float RecoilPitch;

	//Timer

	FTimerHandle TimerHandle_TimeBetweenShots;

	//Multiplayer
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();


	

public:

	void SpawnClip();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	int32 WeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	int32 ActualAmmoInCharger;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	int32 AmmoChargerSize;

	//Disparos
	void StartReload();
	virtual void StartFire();
	void StopFire();

};
