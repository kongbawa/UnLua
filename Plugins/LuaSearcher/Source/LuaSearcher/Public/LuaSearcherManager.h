// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class FLuaPathSearcher;
/**
 * 
 */
class LUASEARCHER_API FLuaSearcherManager
{
public:
    static TSharedPtr<FLuaSearcherManager> Get()
    {
	    return SearcherManager;
    }

    static void SetActive(bool bActive)
    {
        if (bActive)
        {
            if (SearcherManager == nullptr)
            {
                SearcherManager = MakeShared<FLuaSearcherManager>();
            }
        }
        else
        {
            SearcherManager = nullptr;
        }
    }

    FLuaSearcherManager();
    ~FLuaSearcherManager();

    TSharedPtr<FLuaPathSearcher> GetPathSearcher() { return PathSearcher; }

private:
    TSharedPtr<FLuaPathSearcher> PathSearcher;

    static TSharedPtr<FLuaSearcherManager> SearcherManager;
};
