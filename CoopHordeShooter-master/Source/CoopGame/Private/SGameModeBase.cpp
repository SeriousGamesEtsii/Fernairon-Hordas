// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameModeBase.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "SGameState.h"
#include "SCharacter.h"

int32 ASGameModeBase::GetNumRound()
{
	return NumRound;
}


ASGameModeBase::ASGameModeBase()
{
	TimeBetweenWaves = 2.f;

	GameStateClass = ASGameState::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0F;

	NumRound = 0;
}

void ASGameModeBase::StartWave()
{
	WaveCount++;

	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameModeBase::SpawnBotTimerElapsed, 1.f, true, 0.f);

	SetWaveState(EWaveState::WaveInProgress);

}

void ASGameModeBase::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}

}


void ASGameModeBase::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);

}

void ASGameModeBase::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameModeBase::StartWave, TimeBetweenWaves , false);

	NumRound = NumRound + 1;


	SetWaveState(EWaveState::WaitingToStart);
}

void ASGameModeBase::CheckWaveState()
{

	bool bIsPreparingFowWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NrOfBotsToSpawn > 0 || bIsPreparingFowWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();

		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent> (TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.0f && HealthComp->TeamNum != 0)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);
		PrepareForNextWave();
	}
	
}

void ASGameModeBase::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			//ensure rompe el código si devuelve falso. En este caso siempre debería dar verdadero, si no lo hiciese es que hay algo que falla
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				//El jugador sigue vivo
				return;
			}
		}
	}

	//Ningún jugador vivo
	GameOver();

}

void ASGameModeBase::GameOver()
{
	EndWave();

	//@TODO Terminar la partida

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! HABEIS MUERTO, PAQUETES"))
}

void ASGameModeBase::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensure(GS))
	{
		GS->SetWaveState(NewState);
	}
}



void ASGameModeBase::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();

}

void ASGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckWaveState();
	CheckAnyPlayerAlive();

}
