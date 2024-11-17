#pragma once

#include "Reksi/Base.h"
#include "Reksi/ResourceData.h"
#include "Reksi/Resource.h"

namespace Reksi
{
	class ResourceManager
	{
	public:
		ResourceManager(std::filesystem::path basePath);

		bool IsValid(ResourceHandleT handle) const;
		ResourceHandleT GetHandle(const std::filesystem::path& path) const;
		std::type_index GetTypeIndex(ResourceHandleT handle) const;
		template <typename T>
		Resource<T> GetResource(const std::filesystem::path& path, const ResourceLoadFunc<T>& loader);
		template <typename T>
		Resource<T> GetResource(const std::filesystem::path& path);

		template <typename T>
		void DeleteResource(const Resource<T>& resource);
		void MarkForDelete(ResourceHandleT handle);

		template <typename T>
		SharedPtr<T> GetDefaultResource() const;
		template <typename T>
		void SetDefaultResource(const SharedPtr<T>& resource);
		template <typename T>
		ResourceLoadFunc<T> GetDefaultLoader() const;
		template <typename T>
		void SetDefaultLoader(const ResourceLoadFunc<T>& loader);
		// Uses the constructor for default loader
		template <typename T>
		void SetDefaultLoader();

		void Reload(ResourceHandleT handle);

	private:
		std::filesystem::path m_BasePath;
		std::unordered_map<ResourceHandleT, UniquePtr<ResourceData>> m_Resources;
		std::unordered_map<std::filesystem::path, ResourceHandleT> m_ResourcePaths;
		std::vector<uint64_t> m_ValidityMask;

		std::unordered_map<std::type_index, SharedPtr<void>> m_DefaultResources;
		std::unordered_map<std::type_index, ResourceData::LoadFunc> m_DefaultLoaders;

		ResourceHandleT m_NextHandle;

		REKSI_THREADING_MUTABLE REKSI_MUTEX_AUTO;
		// Mutex for the validity mask
		REKSI_THREADING_MUTABLE REKSI_MUTEX(m_ValidMaskMutex);
		// Mutex for default resources and loaders
		REKSI_THREADING_MUTABLE REKSI_MUTEX(m_LoaderResourceMutex);

		bool GetValidityImpl(ResourceHandleT handle) const;
		void SetValidityImpl(ResourceHandleT handle, bool valid);
		template <typename T>
		ResourceData::LoadFunc GetDefaultLoaderImpl() const;
	};
}

#pragma region Defer
namespace Reksi
{
	inline ResourceManager::ResourceManager(std::filesystem::path basePath)
		: m_BasePath(std::move(basePath)), m_ValidityMask({0}), m_NextHandle(1)
	{
	}

	inline bool ResourceManager::IsValid(ResourceHandleT handle) const
	{
		return GetValidityImpl(handle);
	}

	inline ResourceHandleT ResourceManager::GetHandle(const std::filesystem::path& path) const
	{
		REKSI_LOCK_SHARED_AUTO;

		auto itr = m_ResourcePaths.find(path);
		if ( itr == m_ResourcePaths.end() ) return 0;
		return itr->second;
	}

	inline bool ResourceManager::GetValidityImpl(ResourceHandleT handle) const
	{
		REKSI_LOCK_SHARED(m_ValidMaskMutex, lock);

		if ( handle / 64 >= m_ValidityMask.size() || handle == 0 ) return false;
		return m_ValidityMask[handle / 64] & (1ull << (handle % 64));
	}

	inline void ResourceManager::SetValidityImpl(ResourceHandleT handle, bool valid)
	{
		REKSI_LOCK_UNIQUE(m_ValidMaskMutex, lock);

		if ( handle / 64 == m_ValidityMask.size() )
		{
			m_ValidityMask.push_back(0);
		}
		else if ( handle / 64 > m_ValidityMask.size() )
		{
			// resize the vector and set all bits to 0
			m_ValidityMask.resize(handle / 64 + 1, 0);
		}

		// Set the bit to the valid bool
		auto mask = 1ull << (handle % 64);
		m_ValidityMask[handle / 64] = (m_ValidityMask[handle / 64] & (~mask)) | (valid ? mask : 0);
	}

	template <typename T>
	Resource<T> ResourceManager::GetResource(const std::filesystem::path& path, const ResourceLoadFunc<T>& loader)
	{
		REKSI_LOCK_UNIQUE_AUTO;

		// Check if the resource already exists
		auto itr = m_ResourcePaths.find(path);
		if ( itr != m_ResourcePaths.end() )
		{
			auto handle = itr->second;
			ResourceData* data = m_Resources[handle].get();
			return Resource<T>{handle, data, this};
		}

		// Create a new resource
		uint32_t handle = m_NextHandle++;
		ResourceData* data = new ResourceData{handle, m_BasePath / path, loader, this, typeid(T)};
		m_Resources[handle] = UniquePtr<ResourceData>{data};
		m_ResourcePaths[path] = handle;
		SetValidityImpl(handle, true);
		return Resource<T>{handle, data, this};
	}

	template <typename T>
	Resource<T> ResourceManager::GetResource(const std::filesystem::path& path)
	{
		{
			REKSI_LOCK_SHARED_AUTO;

			// Check if the resource already exists
			auto itr = m_ResourcePaths.find(path);
			if ( itr != m_ResourcePaths.end() )
			{
				auto handle = itr->second;
				ResourceData* data = m_Resources[handle].get();
				return Resource<T>{handle, data, this};
			}
		}

		// Need to have the loader, try to get it from the default loaders
		ResourceData::LoadFunc loader = GetDefaultLoaderImpl<T>();
		if ( !loader )
		{
			assert(false);
		}

		{
			REKSI_LOCK_UNIQUE_AUTO;

			// Create the resource
			uint32_t handle = m_NextHandle++;
			auto data = new ResourceData{handle, m_BasePath / path, loader, this, typeid(T)};
			m_Resources[handle] = UniquePtr<ResourceData>{data};
			m_ResourcePaths[path] = handle;
			SetValidityImpl(handle, true);
			return Resource<T>{handle, data, this};
		}
	}

	template <typename T>
	void ResourceManager::DeleteResource(const Resource<T>& resource)
	{
		REKSI_LOCK_UNIQUE_AUTO;

		// Check if resource is already deleted
		if ( !GetValidityImpl(resource.m_Handle) ) return;

		// If not, delete the resource
		// Set validity to false
		SetValidityImpl(resource.m_Handle, false);
		// Remove the resource from the resource paths
		auto path = resource.m_Data->GetPath();
		// Remove the base path from the path
		path = path.string().substr(m_BasePath.string().size());
		m_ResourcePaths.erase(path);
		// Remove the resource from the resources
		m_Resources.erase(resource.m_Handle); // UniquePtr will delete the resource
	}

	inline void ResourceManager::MarkForDelete(ResourceHandleT handle)
	{
		if ( !GetValidityImpl(handle) ) return;

		REKSI_LOCK_UNIQUE_AUTO;

		auto data = m_Resources[handle].get();
		data->SetState(ResourceStatus::MarkedForDelete);
		data->ClearState(ResourceStatus::MarkedForReload);
	}

	inline void ResourceManager::Reload(ResourceHandleT handle)
	{
		if ( !GetValidityImpl(handle) ) ResourceLoadStatus();

		ResourceData* data;
		{
			REKSI_LOCK_UNIQUE_AUTO;
			data = m_Resources[handle].get();
		}
		data->WaitUntilCurrentLoading();
		data->Load();
	}

	template <typename T>
	SharedPtr<T> ResourceManager::GetDefaultResource() const
	{
		REKSI_LOCK_SHARED(m_LoaderResourceMutex, lock);

		auto itr = m_DefaultResources.find(typeid(T));
		if ( itr == m_DefaultResources.end() ) return nullptr;
		return StaticSharedCast<T>(itr->second);
	}

	template <typename T>
	void ResourceManager::SetDefaultResource(const SharedPtr<T>& resource)
	{
		REKSI_LOCK_UNIQUE(m_LoaderResourceMutex, lock);

		m_DefaultResources[typeid(T)] = resource;
	}

	template <typename T>
	ResourceLoadFunc<T> ResourceManager::GetDefaultLoader() const
	{
		auto loader = GetDefaultLoaderImpl<T>();
		if ( !loader ) return nullptr;
		return [loader](const std::filesystem::path& path) -> SharedPtr<T>
		{
			return StaticSharedCast<T>(loader(path));
		};
	}

	template <typename T>
	void ResourceManager::SetDefaultLoader(const ResourceLoadFunc<T>& loader)
	{
		REKSI_LOCK_UNIQUE(m_LoaderResourceMutex, lock);

		m_DefaultLoaders[typeid(T)] = loader;
	}

	template <typename T>
	void ResourceManager::SetDefaultLoader()
	{
		// Assert that T has a constructor that takes a path
		static_assert(std::is_constructible_v<T, const std::filesystem::path&>,
		              "T must have a constructor that takes a path");

		ResourceData::LoadFunc loader = [](const std::filesystem::path& path) -> SharedPtr<T>
		{
			return CreateShared<T>(path);
		};

		REKSI_LOCK_UNIQUE(m_LoaderResourceMutex, lock);
		m_DefaultLoaders[typeid(T)] = loader;
	}

	template <typename T>
	ResourceData::LoadFunc ResourceManager::GetDefaultLoaderImpl() const
	{
		REKSI_LOCK_SHARED(m_LoaderResourceMutex, lock);

		auto itr = m_DefaultLoaders.find(typeid(T));
		if ( itr == m_DefaultLoaders.end() ) return nullptr;
		return itr->second;
	}
}
#pragma endregion
