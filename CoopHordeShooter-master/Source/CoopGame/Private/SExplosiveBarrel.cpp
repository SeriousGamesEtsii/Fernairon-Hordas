// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;
	
	ExplosionImpulse = 600;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	SetReplicateMovement(true);
	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	bExplote = false;

	
}


void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* OwningDamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	FVector impulse;
	impulse.X = FMath::FRandRange(50.f, 200);
	impulse.Y = FMath::FRandRange(50.f, 200);
	impulse.Z = FVector::UpVector.Z * ExplosionImpulse;

	if (Health <= 0 && !bExplote) 
	{
		TArray<AActor*> OverlappingActors;
		OverlappingActors.Add(this);

		bExplote = true;
		OnRep_Exploded();

		UGameplayStatics::ApplyRadialDamage(this, 70, GetActorLocation(), RadialForceComp->Radius, DamageType, OverlappingActors, this, GetInstigatorController(), true);	

		MeshComp->AddImpulse(impulse, NAME_None, true);
		MeshComp->SetMaterial(0, ExplodedMaterial);

		RadialForceComp->FireImpulse();
		PlayExplosion();
	}
}

void ASExplosiveBarrel::PlayExplosion()
{
	FTransform ExplosionTrans;

	ExplosionTrans.SetLocation(GetActorLocation() + FVector(0,0,50));
	ExplosionTrans.SetRotation(GetActorRotation().Quaternion());
	ExplosionTrans.SetScale3D(GetActorScale() *4.0f);

	if (Explosion)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Explosion, ExplosionTrans, true);
	}
	
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	FTransform ExplosionTrans;

	ExplosionTrans.SetLocation(GetActorLocation() + FVector(0, 0, 50));
	ExplosionTrans.SetRotation(GetActorRotation().Quaternion());
	ExplosionTrans.SetScale3D(GetActorScale() *4.0f);

	if (Explosion)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Explosion, ExplosionTrans, true);
	}

	MeshComp->SetMaterial(0, ExplodedMaterial);
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExplote);
}