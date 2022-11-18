// Copyright Epic Games, Inc. All Rights Reserved.

#include "LuaSearcherModule.h"
#include "LuaSearcherSetting.h"

#include "LuaSearcherManager.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "Editor.h"
#endif

#define LOCTEXT_NAMESPACE "FLuaSearcherModule"

void FLuaSearcherModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITOR
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if (!SettingsModule)
        return;

    SettingsModule->RegisterSettings("Project", "Plugins", "LuaSearcher",
                                                    LOCTEXT("LuaSearcherSettingsName", "LuaSearcher"),
                                                    LOCTEXT("LuaSearcherSettingsDescription", "LuaSearcher Runtime Settings"),
                                                    GetMutableDefault<ULuaSearcherSetting>());

    if (!IsRunningGame())
    {
        FEditorDelegates::PreBeginPIE.AddRaw(this, &FLuaSearcherModule::OnPreBeginPIE);
        FEditorDelegates::EndPIE.AddRaw(this, &FLuaSearcherModule::OnEndPIE);
    }

#endif

#if WITH_EDITOR
    if (IsRunningGame() || IsRunningDedicatedServer())
#endif
        FLuaSearcherManager::SetActive(true);
}

void FLuaSearcherModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_EDITOR
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if (SettingsModule)
        SettingsModule->UnregisterSettings("Project", "Plugins", "LuaSearcher");
#endif

    FLuaSearcherManager::SetActive(false);
}

void FLuaSearcherModule::OnPreBeginPIE(bool bIsSimulating)
{
    FLuaSearcherManager::SetActive(false);
    FLuaSearcherManager::SetActive(true);
}

void FLuaSearcherModule::OnEndPIE(bool bIsSimulating)
{
    FLuaSearcherManager::SetActive(false);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLuaSearcherModule, LuaSearcher)