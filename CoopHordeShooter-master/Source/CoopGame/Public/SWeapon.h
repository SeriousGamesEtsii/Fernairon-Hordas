// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* MeshComp;

	//Dispersión de balas en grados
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (ClampMin = 0.0f))
	float BulletSpread;

	void PlayFireEffect(FVector TracerEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditAnywhere, Category = Weapon)
	float BaseDamage;

	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	//RPM - Bullets per minute fired
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float RateOfFire;

	//Derived from rate of fire
	float TimeBetweenShots;

	float RecoilYaw;

	float RecoilPitch;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	float GetBaseDamage();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetBaseDamage(float NewBaseDamage);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

};
