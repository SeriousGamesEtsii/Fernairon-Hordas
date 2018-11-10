#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SActorWidgetComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "AIController.h"
#include "SCharacter.h"


int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw debug lines for weapons"), ECVF_Cheat);


// Sets default values
ASWeapon::ASWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	ActorWidgetComp = CreateDefaultSubobject<USActorWidgetComponent>(TEXT("WidgetActorComp"));
	CapComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	RootComponent = MeshComp;
	ActorWidgetComp->SetupAttachment(RootComponent);
	CapComp->SetupAttachment(RootComponent);
	CapComp->SetRelativeRotation(FRotator(0, 0, 90));

	MuzzleSocketName = "MuzzleSocket";

	ShellEjectSocketName = "ShellEjectionSocket";

	ClipSocketName = "ClipSocket";

	TracerTargetName = "Target";

	WeaponType = 0;

	BaseDamage = 20.0f;

	BulletSpread = 0.0f;

	RateOfFire = 600.0f;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	AmmoChargerSize = 40;

	ActualAmmoInCharger = AmmoChargerSize;

	TotalAmmo = 500;

	Ammunition = TotalAmmo;

	bCanReload = false;

	MaxRangeYaw = 0.0;
	MinRangeYaw = -0.28;

	MaxRangePitch = 0.25f;
	MinRangePitch = -0.25f;

	bIsEquiped = false;
}


void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;
}


void ASWeapon::StartReload()
{
	if (ActualAmmoInCharger >= 40)
	{
		return;
	}
	int32 ClipDelta = FMath::Min(AmmoChargerSize - ActualAmmoInCharger, Ammunition - ActualAmmoInCharger);

	if (ClipDelta > 0)
	{
		ActualAmmoInCharger += ClipDelta;
		Ammunition = Ammunition - ClipDelta;
	}

	//SpawnClip();

}


void ASWeapon::SpawnClip()
{

	FVector EyeLocation;
	FRotator EyeRotation;

	ASCharacter* MyOwner = Cast<ASCharacter>(GetOwner());

	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	//Spawnea el cargador 
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector ClipEjectLocation = MeshComp->GetSocketLocation(ClipSocketName);
	//Spawneamos el clip al recargar
	GetWorld()->SpawnActor<AActor>(ClipClass, ClipEjectLocation, EyeRotation, SpawnParams);
}

void ASWeapon::Fire()
{
	bCanReload = true;

	ASCharacter* MyOwner = Cast<ASCharacter>(GetOwner());

	if (ActualAmmoInCharger <= 0)
	{
		StopFire();
		MyOwner->StartReload();
		return;
	}

	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	ActualAmmoInCharger = ActualAmmoInCharger - 1;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::FString("Actual ammo") + FString::SanitizeFloat(ActualAmmoInCharger));

	if (MyOwner)
	{
		RecoilPitch = FMath::FRandRange(MinRangePitch, MaxRangePitch);
		RecoilYaw = FMath::FRandRange(MinRangeYaw, MaxRangeYaw);

		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);


		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		//Particle "Target"
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		//Trace the world, from pawn eyes to crosshair location
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//Blocking hit! Process damage
			AActor *HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.5f;
			}


			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

			
			
		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Orange, false, 1.0f, 0, 1.0f);
		}

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		PlayFireEffect(TracerEndPoint);

		LastFireTime = GetWorld()->TimeSeconds;

		if (BulletClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FVector ShellEjectLocation = MeshComp->GetSocketLocation(ShellEjectSocketName);
			FRotator BulletRotator = EyeRotation.Add(0, -90, 0);
			//Spawneamos el proyectil al disparar
			GetWorld()->SpawnActor<AActor>(BulletClass, ShellEjectLocation, BulletRotator, SpawnParams);

			
		}

		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->AddPitchInput(RecoilPitch);
			PC->AddYawInput(RecoilYaw);
		}

	}

}



void ASWeapon::OnRep_HitScanTrace()
{
//Play cosmetic FX
	PlayFireEffect(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}



void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::StartFire()
{
	if (ActualAmmoInCharger <= 0)
	{

		return;
	}

	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffect(FVector TracerEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEnd);
		}

	}

	ASCharacter* MyOwner = Cast<ASCharacter>(GetOwner());

	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());

		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}


void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}


	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();


		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}



void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}