// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LuaSearcherSetting.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, Meta=(DisplayName="LuaSearcher"))
class LUASEARCHER_API ULuaSearcherSetting : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere, Category="Server")
    TArray<FString> SerchPaths;
};
