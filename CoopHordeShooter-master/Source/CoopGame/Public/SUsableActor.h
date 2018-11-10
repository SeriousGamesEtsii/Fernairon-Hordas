// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SUsableActor.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASUsableActor : public AActor
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite)
	bool bIsFocused;

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite, meta = (MultiLine = "true"))
	FText Descripcion;

public:

	UFUNCTION(BlueprintImplementableEvent)
	bool OnUsed(ASCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent)
	bool StartFocusItem();

	UFUNCTION(BlueprintImplementableEvent)
	bool EndFocusItem();
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsEquiped;
	
};
