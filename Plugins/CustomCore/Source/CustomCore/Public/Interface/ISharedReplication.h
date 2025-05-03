// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "ISharedReplication.generated.h"

UINTERFACE()
class USharedReplication : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CUSTOMCORE_API ISharedReplication
{
	GENERATED_BODY()

public:
	virtual bool UpdateSharedReplication() =0;
};
