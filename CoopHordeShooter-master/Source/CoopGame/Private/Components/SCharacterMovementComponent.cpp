// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterMovementComponent.h"
#include "SCharacter.h"

float USCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const ASCharacter* CharOwner = Cast<ASCharacter>(PawnOwner);
	if (CharOwner)
	{
		// Slow down during targeting or crouching
		if (CharOwner->IsTargeting() && !IsCrouching())
		{
			MaxSpeed *= CharOwner->GetTargetingSpeedModifier();
		}
	}

	return MaxSpeed;
}


