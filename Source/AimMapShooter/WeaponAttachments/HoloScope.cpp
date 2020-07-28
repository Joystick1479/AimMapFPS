// Fill out your copyright notice in the Description page of Project Settings.
#include "HoloScope.h"


// Sets default values
AHoloScope::AHoloScope()
{

	SetReplicates(true);
}

FName AHoloScope::GetAttachPoint() const
{
	return "ScopeSocket";
}




