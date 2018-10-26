// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = FX)
	class UParticleSystem* Explosion;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Barrel)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Barrel)
	class URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = FX)
	class UMaterialInterface* ExplodedMaterial;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = FX)
	float ExplosionImpulse;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* OwningDamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	UPROPERTY(ReplicatedUsing = OnRep_Exploded)
	bool bExplote;

	UFUNCTION()
	void OnRep_Exploded();


	void PlayExplosion();

	



public:	




};
