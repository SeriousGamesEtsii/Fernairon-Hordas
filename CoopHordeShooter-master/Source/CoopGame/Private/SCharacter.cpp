// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "SCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


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

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	TargetingSpeedModifier = 0.5;

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;
	bDied = false;

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
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);


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
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{

	if (CurrentWeapon)
	{
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

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);

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


	//Challenge code
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
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);

}