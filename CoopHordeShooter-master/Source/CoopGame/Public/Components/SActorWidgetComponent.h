// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "SActorWidgetComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (LODZERO), meta = (BlueprintSpawnableComponent))
class COOPGAME_API USActorWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	virtual void InitWidget() override;

	USActorWidgetComponent();

};