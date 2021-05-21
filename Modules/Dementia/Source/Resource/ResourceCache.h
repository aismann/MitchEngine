#pragma once
#include <map>
#include <string>
#include <Resource/Resource.h>

#include <ClassTypeId.h>
#include <CLog.h>
#include <Path.h>
#include <Pointers.h>
#include <Singleton.h>
#include "MetaFile.h"

class Resource;

class ResourceCache
{
	ResourceCache();
	~ResourceCache();

public:
	std::size_t GetCacheSize() const;

	template<class T, typename... Args>
	SharedPtr<T> Get(const Path& InFilePath, Args&& ... args);

	SharedPtr<Resource> GetCached(const Path& InFilePath);

	void TryToDestroy(Resource* resource);

	const std::map<std::string, std::shared_ptr<Resource>>& GetResouceStack() const;

	void Dump();

	MetaBase* LoadMetadata(const Path& filePath);

private:
	std::map<std::string, std::shared_ptr<Resource>> ResourceStack;

	ME_SINGLETON_DEFINITION(ResourceCache)
};

template<class T, typename... Args>
SharedPtr<T> ResourceCache::Get(const Path& InFilePath, Args&& ... args)
{
	auto I = ResourceStack.find(InFilePath.FullPath);
	if (I != ResourceStack.end())
	{
		SharedPtr<T> Res = std::dynamic_pointer_cast<T>(I->second);
		return Res;
	}

	MetaBase* metaFile = LoadMetadata(InFilePath);
#if ME_EDITOR
	if (metaFile && metaFile->FlaggedForExport)
	{
		metaFile->Export();
		metaFile->Save();
	}
#endif

	if (!InFilePath.Exists && !metaFile->FlaggedForExport)
	{
		YIKES("Failed to load resource: " + InFilePath.FullPath);
		return {};
	}

	SharedPtr<T> Res = std::make_shared<T>(InFilePath, std::forward<Args>(args)...);
	Res->Resources = this;
	TypeId id = ClassTypeId<Resource>::GetTypeId<T>();
	Res->ResourceType = static_cast<std::size_t>(id);
	Res->SetMetadata(metaFile);
	Res->Load();
	ResourceStack[InFilePath.FullPath] = Res;
	return Res;
}
