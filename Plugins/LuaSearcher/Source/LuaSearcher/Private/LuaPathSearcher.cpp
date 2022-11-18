#include "LuaPathSearcher.h"
#include "Engine.h"
#include "HAL/FileManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogLuaPathSearcher, Log, All);

struct FLuaPathSearcher::FSearcherBase
{
public:
    FSearcherBase(const FString& InputContentPath, const FString& InLuaFileExtension)
        : ContentPath(InputContentPath)
        , LuaFileExtension(InLuaFileExtension)
    {
        RootPath = FPaths::RootDir();
        if (ContentPath.Find(RootPath) == INDEX_NONE)
        {
            RootPath.Empty();
        }
    }
    virtual ~FSearcherBase() {};
    virtual void AddPath(const FString& Path) = 0;
    virtual void Clear() = 0;
    virtual FString FindFullPath(const FString& ScriptName) = 0;
    virtual bool IsFileExisted(const FString& ScriptName) = 0;
    virtual void SetExternalFiles(const TArray<FString>& Files) = 0;

    FString ContentPath;
    FString LuaFileExtension;
    FString RootPath;
};

//////////////////////////////////////////////////////////////////////////
static const int ScriptExtensionNum = 2;
static const FString ScriptExtensions[ScriptExtensionNum] = { TEXT(".lua"), TEXT(".luac") };
static const int LuaExtensionLen[ScriptExtensionNum] = { ScriptExtensions[0].Len(), ScriptExtensions[1].Len() };

struct FLuaPathSearcher::FIteratePath : public FLuaPathSearcher::FSearcherBase
{
    TArray<FString> SearchPaths;
    int CurrentPathIndex;
    TMap<FName, int> FileNameToPaths;
    TMap<FName, FString> ExternalFiles;

    FIteratePath(const FString& InputRootPath, const FString& InLuaFileExtension)
        : FSearcherBase(InputRootPath, InLuaFileExtension)
        , CurrentPathIndex(0)
    {
    }

    virtual void AddPath(const FString& InputPath) override
    {
        FString Path = ContentPath / InputPath;
        class FAddLuaSearchPathVisitor : public IPlatformFile::FDirectoryVisitor
        {
        public:
            FAddLuaSearchPathVisitor(FIteratePath* pTemp)
                : pPaths(pTemp)
            {}

            virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
            {
                FString FileOrPath = FilenameOrDirectory;
                if (pPaths->RootPath.Len() > 0 && FileOrPath.Find(pPaths->RootPath) == INDEX_NONE)
                {
                    FileOrPath = pPaths->RootPath + FileOrPath;
                }
                //UE_LOG(LogTemp, Log, TEXT("FLuaPathSearcher Visit pPaths->RootPath %s  FilenameOrDirectory %s  FileOrPath %s"), *pPaths->RootPath, FilenameOrDirectory, *FileOrPath);
                // 这里必定先visit dir然后在遍历dir底下的文件
                if (bIsDirectory)
                {
                    pPaths->AddDir(FileOrPath);
                }
                else
                {
                    pPaths->AddFile(FileOrPath);
                }
                return true;
            }

        private:
            FIteratePath * pPaths;
        } PathVisitor(this);

        AddDir(Path);
        IFileManager::Get().IterateDirectoryRecursively(*Path, PathVisitor);
    }

    virtual void Clear() override
    {
        SearchPaths.Empty();
        CurrentPathIndex = 0;
        FileNameToPaths.Empty();
    }

    virtual FString FindFullPath(const FString& ScriptName) override
    {
        if (ExternalFiles.Num() > 0)
        {
            auto Ret = ExternalFiles.Find(*ScriptName);
            if (Ret)
            {
                return *Ret;
            }
        }

        int* pTemp = FileNameToPaths.Find(*ScriptName);
        if (pTemp)
        {
            return SearchPaths[*pTemp] / ScriptName + LuaFileExtension;
        }
        return FString();
    }

    virtual bool IsFileExisted(const FString& ScriptName) override
    {
        return FileNameToPaths.Find(*ScriptName) != nullptr;
    }

    virtual void SetExternalFiles(const TArray<FString>& Files) override
    {
        ExternalFiles.Empty(Files.Num());
        for (auto& FilePath : Files)
        {
            ExternalFiles.Emplace(*FPaths::GetBaseFilename(FilePath), FilePath);
        }
    }

    void AddDir(const FString& Path)
    {
        FString DirPath = Path;
        if (!DirPath.EndsWith(TEXT("/")))
        {
            DirPath.AppendChar('/');
        }

        if (SearchPaths.Contains(DirPath))
            return;

        //UE_LOG(LogTemp, Log, TEXT("FLuaPathSearcher AddDir %s "), *DirPath);
        CurrentPathIndex = SearchPaths.Num();
        SearchPaths.Add(DirPath);
    }

    void AddFile(const FString& Path)
    {
        FString FileExtension = FPaths::GetExtension(Path, true);
        if (FileExtension.Compare(LuaFileExtension) != 0 || !IFileManager::Get().FileExists(*Path))
        {
            return;
        }
        // 不知道为什么先加的文件后加的文件夹
        FString Dir = FPaths::GetPath(Path).AppendChar('/');
        if (Dir != SearchPaths[CurrentPathIndex])
        {
            AddDir(Dir);
        }
        
        if (Dir != SearchPaths[CurrentPathIndex])
        {
            if (!SearchPaths.Find(Dir, CurrentPathIndex))
            {
                UE_LOG(LogLuaPathSearcher, Error, TEXT("AddFilePath failed, %s"), *Path);
                return;
            }
        }
        FName KeyName(*FPaths::GetBaseFilename(Path));
        //UE_LOG(LogTemp, Log, TEXT("FLuaPathSearcher AddFile %s %s"), *Dir, *KeyName.ToString());
        checkf(!FileNameToPaths.Contains(KeyName), TEXT("Duplicated lua file: %s"), *Path);
        FileNameToPaths.Add(KeyName, CurrentPathIndex);
    }
};

FLuaPathSearcher::FLuaPathSearcher(const FString& ContentPath, const FString& LuaFileExtension)
{
    Init(ContentPath, LuaFileExtension);
}
FLuaPathSearcher::~FLuaPathSearcher()
{
    Uninit();
}

//////////////////////////////////////////////////////////////////////////
bool FLuaPathSearcher::Init(const FString& ContentPath, const FString& LuaFileExtension)
{
    Impl = MakeShareable(new FIteratePath(ContentPath, LuaFileExtension));

    //auto Settings = GetMutableDefault<UUnLuaSettings>();
    ////WorldContextObject->GetWorld();
    //if (!UKismetSystemLibrary::IsDedicatedServer(GWorld))
    //{
    //    SetSearchPath(Settings->ClientSerchPaths);
    //}
    //else
    //{
    //    SetSearchPath(Settings->ServerSerchPaths);
    //}
    return true;
}

void FLuaPathSearcher::Uninit()
{
    Clear();
}

void FLuaPathSearcher::SetExternalFiles(const TArray<FString>& Files)
{
    Impl->SetExternalFiles(Files);
}

void FLuaPathSearcher::SetSearchPath(const TArray<FString>& Paths)
{
    Clear();
    for(int ii=0; ii<Paths.Num(); ++ii)
    {
        AddPath(Paths[ii]);
    }
}


void FLuaPathSearcher::AddPath(const FString& Path)
{
    Impl->AddPath(Path);
}

void FLuaPathSearcher::Clear()
{
    if(Impl.IsValid())
    {
        Impl->Clear();
    }
}

FString FLuaPathSearcher::FindFullPath(const FString& ScriptName)
{
    return Impl->FindFullPath(ScriptName);
}

bool FLuaPathSearcher::IsFileExisted(const FString& Path)
{
    return Impl->IsFileExisted(Path);
}
