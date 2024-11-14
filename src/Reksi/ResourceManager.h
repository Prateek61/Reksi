#pragma once

#include "Reksi/Base.h"
#include "Reksi/ResourceData.h"
#include "Reksi/Resource.h"

namespace Reksi
{
	class ResourceManager
	{
	public:
		ResourceManager();

		template <typename T>
		Resource<T> GetResource(const std::filesystem::path& path, const ResourceData::LoaderFunc& loader,
		                        bool immediateLoad = false);
		template <typename T>
		Resource<T> GetResource(const std::filesystem::path& path, bool immediateLoad = false);
		template <typename T>
		Resource<T> GetResource(ResourceHandleType handle);

	private:
		ResourceHandleType m_CurrHandle;
		std::unordered_map<ResourceHandleType, SharedPtr<ResourceData>> m_Resources;
		std::unordered_map<std::filesystem::path, ResourceHandleType> m_ResourcePaths;
		RK_AUTO_MUTEX
	};
}

// Implementation
namespace Reksi
{
	inline ResourceManager::ResourceManager()
		: m_CurrHandle(1)
	{
	}

	template <typename T>
	Resource<T> ResourceManager::GetResource(const std::filesystem::path& path, const ResourceData::LoaderFunc& loader,
	                                         bool immediateLoad)
	{
		ResourceHandleType handle = 0;
		{
			RK_AUTO_LOCK_SCOPE

			auto itr = m_ResourcePaths.find(path);
			if ( itr != m_ResourcePaths.end() )
			{
				handle = itr->second;
			}
		}

		if ( handle != 0 )
		{
			Resource<T> resource = GetResource<T>(handle);

			if ( immediateLoad ) resource.Load();
			return resource;
		}

		// Create new Resource Data
		SharedPtr<ResourceData> data(new ResourceData(path, loader));

		{
			RK_AUTO_LOCK_SCOPE

			handle = m_CurrHandle++;
			m_Resources[handle] = data;
			m_ResourcePaths[path] = handle;
		}

		if ( immediateLoad ) data->Load();

		return Resource<T>{handle, data};
	}

	template <typename T>
	Resource<T> ResourceManager::GetResource(const std::filesystem::path& path, bool immediateLoad)
	{
		ResourceHandleType handle = 0;
		{
			RK_AUTO_LOCK_SCOPE

			auto itr = m_ResourcePaths.find(path);
			if ( itr != m_ResourcePaths.end() )
			{
				handle = itr->second;
			}
		}

		if ( handle != 0 )
		{
			Resource<T> resource = GetResource<T>(handle);

			if ( immediateLoad ) resource.Load();
			return resource;
		}

		// Check if the resource has a constructor that takes a path
		static_assert(std::is_constructible_v<T, const std::filesystem::path&>,
		              "Resource type must have a constructor that takes a const std::filesystem::path&");

		// Use the constructor of the T type to create a loader
		ResourceData::LoaderFunc loader = [path](const std::filesystem::path& p)
		{
			return CreateShared<T>(p);
		};
		// Create the resource
		SharedPtr<ResourceData> data(new ResourceData(path, loader));

		{
			RK_AUTO_LOCK_SCOPE

			handle = m_CurrHandle++;
			m_Resources[handle] = data;
			m_ResourcePaths[path] = handle;
		}

		if ( immediateLoad ) data->Load();
		return Resource<T>{handle, data};
	}

	template <typename T>
	Resource<T> ResourceManager::GetResource(ResourceHandleType handle)
	{
		RK_AUTO_LOCK_SCOPE

		return Resource<T>{handle, m_Resources[handle]};
	}
}
