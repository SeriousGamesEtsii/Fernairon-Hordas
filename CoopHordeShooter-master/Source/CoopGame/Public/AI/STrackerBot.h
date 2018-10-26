// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	class USHealthComponent* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	class USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


	void GetNextPathPoint();



	FVector NextPathPoint;


	UPROPERTY(EditDefaultsOnly, Category = TrackerBot)
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = TrackerBot)
	bool bUseVelocityChange;

	bool bInterruptor;

	void ChangeInterruptor();

	UPROPERTY(EditDefaultsOnly, Category = TrackerBot)
	float RequiredDistanceToTarget;

	class UMaterialInstanceDynamic* MatInst;

	bool bExplode;

	UPROPERTY(EditDefaultsOnly, Category = FX)
	class UParticleSystem* Explosion;

	void SelfDestruct();

	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	int32 PowerLevel;

	float Radius;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	int32 MaxPowerLevel;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float DamageRadius;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;

	FTimerHandle TimerHandle_CheckPowerLevel;

	FTimerHandle TimerHandle_CheckPlayer;

	void SelfDamage();

	bool bStarterSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = FX)
	class USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = FX)
	class USoundCue* ExplodeSound;

	void OnCheckNearbyBots();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
};
