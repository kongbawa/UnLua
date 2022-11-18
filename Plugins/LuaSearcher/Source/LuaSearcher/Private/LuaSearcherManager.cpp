// Fill out your copyright notice in the Description page of Project Settings.


#include "LuaSearcherManager.h"
#include "LuaSearcherSetting.h"
#include "LuaPathSearcher.h"

TSharedPtr<FLuaSearcherManager> FLuaSearcherManager::SearcherManager;

FLuaSearcherManager::FLuaSearcherManager()
{
    PathSearcher = MakeShared<FLuaPathSearcher>(FPaths::ProjectContentDir(), TEXT(".lua"));
    auto Settings = GetMutableDefault<ULuaSearcherSetting>();

    PathSearcher->SetSearchPath(Settings->SerchPaths);
}

FLuaSearcherManager::~FLuaSearcherManager()
{
    PathSearcher->Uninit();
    PathSearcher = nullptr;
}

