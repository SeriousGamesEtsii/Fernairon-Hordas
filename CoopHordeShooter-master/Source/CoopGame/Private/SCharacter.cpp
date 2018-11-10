// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "CoopGame.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "SActorWidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "SCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SUsableActor.h"
#include "AIController.h"


// Sets default values
ASCharacter::ASCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	// Adjust jump to make it less floaty
	MoveComp->GravityScale = 1.5f;
	MoveComp->JumpZVelocity = 620;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->MaxWalkSpeedCrouched = 200;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	WidgetComp = CreateDefaultSubobject<USActorWidgetComponent>(TEXT("WidgetComponent"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;
	WidgetComp->SetupAttachment(SpringArmComp);
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	TargetingSpeedModifier = 0.5;

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;
	bDied = false;
	bIsFiring = false;

	MaxUseDistance = 550;
	bHasNewFocus = true;

	NumMaxCompanion = 1;
	CurrentCompanion = 0;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();



	DefaultFOV = CameraComp->FieldOfView;

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);


	if (Role == ROLE_Authority)
	{
		//Spawn a default weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (CurrentWeapon)
		{

			CurrentWeapon->SetOwner(this);
			SelectSocketWeapon();
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
		}
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentWeapon->bIsEquiped = bIsReloading;

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);

	if ((Controller && Controller->IsLocalController()))
	{
		ASUsableActor* Usable = GetUsableInView();

		//Termina el foco
		if (FocusedUsableActor != Usable)
		{
			if (FocusedUsableActor)
			{
				FocusedUsableActor->EndFocusItem();
			}

			bHasNewFocus = true;
		}

		FocusedUsableActor = Usable;

		//Empieza el foco

		if (Usable)
		{
			if (bHasNewFocus)
			{
				Usable->StartFocusItem();
				bHasNewFocus = false;
			}
		}
	}

}

void ASCharacter::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() * value);
}


void ASCharacter::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector() * value);
}

float ASCharacter::PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate, FName StartSectionName)
{
	float Duration = 0.0f;
	
	if (Animation)
	{
		Duration = PlayAnimMontage(Animation, InPlayRate, StartSectionName);
	}
	

	return Duration;
}

void ASCharacter::StopWeaponAnimation(UAnimMontage* Animation)
{
	if (CurrentWeapon)
	{
		if (Animation)
		{
			this->StopAnimMontage(Animation);
		}
	}
}

void ASCharacter::StartReload()
{
	/*if (Role < ROLE_Authority)
	{
		ServerStartReload();
	}*/
	if (CurrentWeapon && CurrentWeapon->ActualAmmoInCharger < CurrentWeapon->AmmoChargerSize && !bIsReloading)
	{
		if (CanReload())
		{
			bIsReloading = true;
			StopFire();
			float AnimDuration = PlayWeaponAnimation(ReloadAnim);
			if (AnimDuration <= 0.0f)
			{
				AnimDuration = 0;
			}

			GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &ASCharacter::StopSimulateReload, AnimDuration, false);
			if (Role == ROLE_Authority)
			{
				CurrentWeapon->SpawnClip();
				GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ASCharacter::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
			}
			/*if (This->IsLocallyControlled())
			{
				PlayWeaponSound(ReloadSound);
			}*/
		}
	}
}

void ASCharacter::StopSimulateReload()
{
	if (CurrentWeapon)
	{
		bIsReloading = false;
		StopWeaponAnimation(ReloadAnim);
		APlayerController* PC =	Cast<APlayerController>(GetController());
		if (PC)
		{
			FKey Key = EKeys::LeftMouseButton;

			if (PC->IsInputKeyDown(Key))
			{
				StartFire();
			}
		}
	}
}

void ASCharacter::ReloadWeapon()
{
	if(CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}

}

void ASCharacter::SelectSocketWeapon()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->WeaponType == 0)
		{
			WeaponAttachSocket = "WeaponSocket";
		}
		if (CurrentWeapon->WeaponType == 1)
		{
			WeaponAttachSocket = "WeaponSocketAk47";
		}
		if (CurrentWeapon->WeaponType == 2)
		{
			WeaponAttachSocket = "WeaponSocket";
		}
		if (CurrentWeapon->WeaponType == 3)
		{
			WeaponAttachSocket = "WeaponSocket";
		}

	}
}


bool ASCharacter::CanReload()
{
	return !bDied;
}

ASWeapon* ASCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

bool ASCharacter::IsFiring() const
{
	return bIsFiring;
}

void ASCharacter::SetCurrentCompanion(int32 NewCurrentCompanion)
{
	if (CurrentCompanion >= NumMaxCompanion)
	{
		return;
	}
	CurrentCompanion = NewCurrentCompanion;
}

int32 ASCharacter::GetCurrentCompanion()
{
	return CurrentCompanion;
}


void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

bool ASCharacter::IsTargeting() const
{
	return bIsTargeting;
}

float ASCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}

FRotator ASCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

ASUsableActor * ASCharacter::GetUsableInView()
{
	FVector CamLoc;
	FRotator CamRot;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == NULL)
	{
		return NULL;
	}
	if (PC)
	{
		PC->GetActorEyesViewPoint(CamLoc, CamRot);
	}
	const FVector StartTrace = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + (Direction * MaxUseDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_PROJECTILE, TraceParams);

	//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor(255, 255, 255), false, 1);

	return Cast<ASUsableActor>(Hit.GetActor());
}

void ASCharacter::Use_Implementation()
{
	ASUsableActor* Usable = GetUsableInView();
	if (Usable)
	{
		Usable->OnUsed(this);
	}
}

bool ASCharacter::Use_Validate()
{
	return true;
}

void ASCharacter::SetTargeting(bool NewTargeting)
{
	bIsTargeting = NewTargeting;

	if (Role < ROLE_Authority)
	{
		ServerSetTargeting(NewTargeting);
	}
}

void ASCharacter::ServerSetTargeting_Implementation(bool NewTargeting)
{
	SetTargeting(NewTargeting);
}

bool ASCharacter::ServerSetTargeting_Validate(bool NewTargeting)
{
	return true;
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
	SetTargeting(bWantsToZoom);
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
	SetTargeting(bWantsToZoom);
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon && !bIsReloading)
	{
		bIsFiring = true;
		CurrentWeapon->StartFire();		
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		bIsFiring = false;
		CurrentWeapon->StopFire();		
	}

}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		//Die
		bDied = true;

		GetMovementComponent()->StopMovementImmediately();

		StopFire();

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
	}
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Recharge", IE_Pressed, this, &ASCharacter::StartReload);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &ASCharacter::Use);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
}


FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASCharacter, bIsTargeting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASCharacter, bIsFiring, COND_SkipOwner);
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);


}