#pragma once
#pragma once

#include "CoreMinimal.h"

class LUASEARCHER_API FLuaPathSearcher
{
    struct FSearcherBase;
    struct FIteratePath;
    TSharedPtr<FSearcherBase> Impl;
public:
    FLuaPathSearcher(const FString& ContentPath, const FString& LuaFileExtension);
    ~FLuaPathSearcher();
public:
    bool Init(const FString& ContentPath, const FString& LuaFileExtension);
    void Uninit();
    void SetExternalFiles(const TArray<FString>& Files);
    void SetSearchPath(const TArray<FString>& Paths);
    const FString& GetContentPath() const;
    void AddPath(const FString& Path);
    void Clear();
    FString FindFullPath(const FString& ScriptName);
    bool IsFileExisted(const FString& ScriptName);
};