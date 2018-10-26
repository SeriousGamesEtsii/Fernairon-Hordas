// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameModeBase.generated.h"

enum class EWaveState : uint8;

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	int32 NrOfBotsToSpawn;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = GameMode)
	float TimeBetweenWaves;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = Gamemode)
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	void StartWave();

	void EndWave();

	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	int32 NumRound;

public:

	
	UFUNCTION(BlueprintCallable, Category = GameMode)
	int32 GetNumRound();

	ASGameModeBase();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
};
