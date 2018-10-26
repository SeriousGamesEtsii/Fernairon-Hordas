// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	DamageRadius = 350.f;
	SphereComp->SetSphereRadius(DamageRadius);
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MovementForce = 11600;
	bUseVelocityChange = false;
	RequiredDistanceToTarget = 200;
	BaseDamage = 40.0f;
	Radius = 600;
	SelfDamageInterval = 0.25f;
	PowerLevel = 1;
	MaxPowerLevel = 4;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		//Primer move-to
		GetNextPathPoint();

		GetWorldTimerManager().SetTimer(TimerHandle_CheckPlayer, this, &ASTrackerBot::ChangeInterruptor, 1.6f, true);
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.f, true);
	}
}


void ASTrackerBot::SelfDamage()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, DamageType);
}



void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStarterSelfDestruction && !bExplode)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		ASTrackerBot* Friend = Cast<ASTrackerBot>(OtherActor);

		

		if (PlayerPawn && !HealthComp->IsFriendly(this, PlayerPawn))
		{
			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::SelfDamage, SelfDamageInterval, true, 0.0f);
			}

			bStarterSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
	
}

void ASTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath && NavPath->PathPoints.Num() > 1)
	{
		NextPathPoint = NavPath->PathPoints[1];
	}

	else
	{
		NextPathPoint = GetActorLocation();
	}
	

}

void ASTrackerBot::ChangeInterruptor()
{
	bInterruptor = true;
}

void ASTrackerBot::SelfDestruct()
{

	MeshComp->SetVisibility(false, true);

	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float ActualDamage = BaseDamage + (BaseDamage * PowerLevel);

		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), DamageRadius, DamageType, IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 12, FColor::Blue, false, 2.f, 0, 1.f);

		SetLifeSpan(2.0f);
	}
	

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Explosion, GetActorLocation());

	

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//Explotar en el objetivo == 0
	if (Health <= 0 && !bExplode)
	{
		bExplode = true;
		SelfDestruct();
	}

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())

}
void ASTrackerBot::OnCheckNearbyBots()
{
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	FCollisionObjectQueryParams QueryParams;

	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;

	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.f);

	int32 NrOfBots = 0;

	for (FOverlapResult Result : Overlaps)
	{
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());

		if (Bot && Bot != this)
		{
			NrOfBots++;
		}
	}

	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);


	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		float Alpha = PowerLevel / (float)MaxPowerLevel;


		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

}
// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExplode)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget || bInterruptor)
		{
			bInterruptor = false;
			GetNextPathPoint();
			DrawDebugString(GetWorld(), GetActorLocation(), "I see You", this, FColor::Black, 1);
		}

		else
		{
			//Continuamos al siguiente punto
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.f, 0.f, 1.f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.f, 1.f);

	}
	
}
