#pragma once

// -------------- Start of Base.h----------------

// Flag to enable or disable custom smart pointer implementations, uses std::unique_ptr and std::shared_ptr by default
#ifndef RK_CUSTOM_SP_IMPLEMENTATION
#define RK_CUSTOM_SP_IMPLEMENTATION 0
#endif

// Flag to enable or disable custom mutex implementations, uses std::mutex by default
#ifndef RK_CUSTOM_MUTEX_IMPLEMENTATION
#define RK_CUSTOM_MUTEX_IMPLEMENTATION 0
#endif

// Thread Safety
#ifndef RK_THREAD_SAFE
#define RK_THREAD_SAFE 1
#endif

#if RK_CUSTOM_MUTEX_IMPLEMENTATION == 0
#if RK_THREAD_SAFE
#include <mutex>
#define RK_AUTO_MUTEX std::mutex Mutex;
#define RK_AUTO_LOCK_SCOPE std::lock_guard<std::mutex> lock(Mutex);
#define RK_MUTEX(x) std::mutex x;
#define RK_LOCK_SCOPE(x) std::lock_guard<std::mutex> lock(x);
#else
#define RK_AUTO_MUTEX
#define RK_AUTO_LOCK_SCOPE
#define RK_MUTEX(x)
#define RK_LOCK_SCOPE(x)
#endif
#endif

#if RK_CUSTOM_SP_IMPLEMENTATION == 0
#include <memory>

namespace Reksi
{
	// Unique Pointer
	template <typename T>
	using UniquePtr = std::unique_ptr<T>;

	template <typename T, typename... Args>
	constexpr UniquePtr<T> CreateUnique(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	// Shared Pointer
	template <typename T>
	using SharedPtr = std::shared_ptr<T>;

	template <typename T, typename... Args>
	constexpr SharedPtr<T> CreateShared(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// Static Pointer Cast
	template <typename T, typename U>
	constexpr SharedPtr<T> StaticSharedCast(const SharedPtr<U>& ptr)
	{
		return std::static_pointer_cast<T>(ptr);
	}
}

#endif

// If Mutex macros are not defined, error
#if !defined(RK_AUTO_MUTEX) || !defined(RK_AUTO_LOCK_SCOPE) || !defined(RK_MUTEX) || !defined(RK_LOCK_SCOPE)
#error "One or more Mutex macros are not defined, please define them in Base.h"
#endif

// TODO: If smart pointers are not defined, error

// Include Files
#include <cstdint>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <functional>

// -------------------- End of Base.h------------------


// -------------- Start of ResourceData.h----------------


namespace Reksi
{
	// Forward Declaring ResourceManager
	class ResourceManager;
}

namespace Reksi
{
	enum class ResourceStatus
	{
		NotLoaded,
		Loading,
		Loaded
	};

	class ResourceData
	{
	public:
		using LoaderFunc = std::function<SharedPtr<void>(const std::filesystem::path&)>;

		bool Reload();
		void Load();
		void Unload();
		ResourceStatus GetStatus();
		bool IsReloading();
		bool IsLoaded();
		template <typename T>
		SharedPtr<T> GetData();
		std::filesystem::path GetPath();
		LoaderFunc GetLoader();
		
	private:
		ResourceData(std::filesystem::path path, LoaderFunc loader);
		ResourceData(const ResourceData&) = delete;

		std::filesystem::path m_Path;
		LoaderFunc m_Loader;
		SharedPtr<void> m_Data;
		ResourceStatus m_Status;
		bool m_Reloading;
		RK_AUTO_MUTEX

		friend class ResourceManager;
	};
}

// Implementation
namespace Reksi
{
	inline bool ResourceData::Reload()
	{
		{
			RK_AUTO_LOCK_SCOPE

			if ( m_Reloading || m_Status == ResourceStatus::Loading ) return false;
			if ( m_Status == ResourceStatus::NotLoaded ) m_Status = ResourceStatus::Loading;
			m_Reloading = true;
		}

		auto new_data = m_Loader(m_Path);

		{
			RK_AUTO_LOCK_SCOPE

			m_Data = new_data;
			m_Status = ResourceStatus::Loaded;
			m_Reloading = false;
		}

		return true;
	}

	inline void ResourceData::Load()
	{
		bool is_loading = false;

		{
			RK_AUTO_LOCK_SCOPE

			if ( m_Status == ResourceStatus::Loading ) is_loading = true;
			if (m_Status == ResourceStatus::NotLoaded) m_Status = ResourceStatus::Loading;
			if ( m_Status == ResourceStatus::Loaded ) return;
		}

		if ( is_loading )
		{
			// Wait for the resource to finish loading
			while ( true )
			{
				{
					RK_AUTO_LOCK_SCOPE

					if ( m_Status == ResourceStatus::Loaded ) return;
				}

				std::this_thread::yield();
			}
		}

		auto new_data = m_Loader(m_Path);

		{
			RK_AUTO_LOCK_SCOPE

			m_Data = new_data;
			m_Status = ResourceStatus::Loaded;
		}
	}

	inline void ResourceData::Unload()
	{
		RK_AUTO_LOCK_SCOPE

		m_Data.reset();
		m_Status = ResourceStatus::NotLoaded;
	}

	inline ResourceStatus ResourceData::GetStatus()
	{
		RK_AUTO_LOCK_SCOPE

		return m_Status;
	}

	inline bool ResourceData::IsReloading()
	{
		RK_AUTO_LOCK_SCOPE

		return m_Reloading;
	}

	inline bool ResourceData::IsLoaded()
	{
		RK_AUTO_LOCK_SCOPE

		return m_Status == ResourceStatus::Loaded;
	}

	inline std::filesystem::path ResourceData::GetPath()
	{
		return m_Path;
	}

	inline ResourceData::LoaderFunc ResourceData::GetLoader()
	{
		return m_Loader;
	}

	template <typename T>
	SharedPtr<T> ResourceData::GetData()
	{
		RK_AUTO_LOCK_SCOPE

		return StaticSharedCast<T>(m_Data);
	}

	inline ResourceData::ResourceData(std::filesystem::path path, LoaderFunc loader)
		: m_Path(std::move(path)), m_Loader(std::move(loader)), m_Status(ResourceStatus::NotLoaded), m_Reloading(false)
	{
	}
}

// -------------------- End of ResourceData.h------------------


// -------------- Start of Resource.h----------------


// Forward Declaring ResourceManager
namespace Reksi
{
	class ResourceManager;
}

namespace Reksi
{
	using ResourceHandleType = uint32_t;

	template <typename T>
	class Resource
	{
	public:
		SharedPtr<T> GetRef();
		T& operator*();

		ResourceStatus GetStatus() const;
		bool IsLoaded() const;
		bool IsReloading() const;

		bool Reload();
		void Load();
		void Unload();

		std::filesystem::path GetPath() const;
		ResourceData::LoaderFunc GetLoader() const;

	private:
		Resource(ResourceHandleType handle, const SharedPtr<ResourceData>& data);

		ResourceHandleType m_Handle;
		SharedPtr<ResourceData> m_Data;
		friend class ResourceManager;
	};
}

// Implementation
namespace Reksi
{
	template <typename T>
	SharedPtr<T> Resource<T>::GetRef()
	{
		return m_Data->GetData<T>();
	}

	template <typename T>
	T& Resource<T>::operator*()
	{
		return *m_Data->GetData<T>();
	}

	template <typename T>
	ResourceStatus Resource<T>::GetStatus() const
	{
		return m_Data->GetStatus();
	}

	template <typename T>
	bool Resource<T>::IsLoaded() const
	{
		return m_Data->IsLoaded();
	}

	template <typename T>
	bool Resource<T>::IsReloading() const
	{
		return m_Data->IsReloading();
	}

	template <typename T>
	bool Resource<T>::Reload()
	{
		return m_Data->Reload();
	}

	template <typename T>
	void Resource<T>::Load()
	{
		m_Data->Load();
	}

	template <typename T>
	void Resource<T>::Unload()
	{
		m_Data->Unload();
	}

	template <typename T>
	std::filesystem::path Resource<T>::GetPath() const
	{
		return m_Data->GetPath();
	}

	template <typename T>
	ResourceData::LoaderFunc Resource<T>::GetLoader() const
	{
		return m_Data->GetLoader();
	}

	template <typename T>
	Resource<T>::Resource(ResourceHandleType handle, const SharedPtr<ResourceData>& data)
		: m_Handle(handle), m_Data(data)
	{
	}
}

// -------------------- End of Resource.h------------------


// -------------- Start of ResourceManager.h----------------


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

// -------------------- End of ResourceManager.h------------------

