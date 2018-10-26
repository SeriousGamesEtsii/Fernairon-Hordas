// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"



UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter(const class FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Companion")
	int32 NumMaxCompanion;

	UPROPERTY(EditDefaultsOnly, Category = "Companion")
	int32 CurrentCompanion;

	/*Movement*/
	void MoveForward(float value);

	void MoveRight(float value);

	float SprintingSpeed;
public:

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void SetCurrentCompanion(int32 NewCurrentCompanion);

	UFUNCTION(BlueprintCallable, Category = "Companion")
	int32 GetCurrentCompanion();

protected:
	/* Targeting */

	void SetTargeting(bool NewTargeting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetTargeting(bool NewTargeting);

	void ServerSetTargeting_Implementation(bool NewTargeting);

	bool ServerSetTargeting_Validate(bool NewTargeting);

	UPROPERTY(Transient, Replicated)
	bool bIsTargeting;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetingSpeedModifier;

public:

	/* Is player aiming down sights */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool IsTargeting() const;

	float GetTargetingSpeedModifier() const;

	/* Retrieve Pitch/Yaw from current camera */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	FRotator GetAimOffsets() const;



	/*Components*/

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USHealthComponent* HealthComp;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	//Default fov set during begin play
	float DefaultFOV;

	UPROPERTY(Replicated)
	class ASWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocket = "WeaponSocket";

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//Die
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = Player)
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = Player)
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = Player)
	void BeginZoom();

	UFUNCTION(BlueprintCallable, Category = Player)
	void EndZoom();

	UFUNCTION(BlueprintCallable, Category = Player)
	void BeginCrouch();

	UFUNCTION(BlueprintCallable, Category = Player)
	void EndCrouch();

};