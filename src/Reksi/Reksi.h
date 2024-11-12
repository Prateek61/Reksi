#pragma once

// A header only resource manager
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <cstdint>
#include <filesystem>
#include <functional>

#define RK_THREAD_SAFE 1

#ifndef RK_THREAD_SAFE
#define RK_THREAD_SAFE 0
#endif

#if RK_THREAD_SAFE
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

namespace Reksi
{
    // Memory stuff
    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T1, typename T2>
    constexpr Ref<T1> StaticRefCast(const Ref<T2>& ref)
    {
		return std::static_pointer_cast<T1>(ref);
    }

    class ResourceManager;
    typedef uint32_t ResourceIDType;

    enum class ResourceStatus
    {
        NotLoaded = 0,
        Loading,
        Loaded
    };

    struct ResourceData
    {
        using LoadFunc = std::function<Ref<void>(const std::filesystem::path&)>;

        const std::filesystem::path Path;
        Ref<void> Data;
        ResourceStatus Status;
        bool Reloading = false;
        const LoadFunc Loader;

        RK_AUTO_MUTEX

        bool Reload();
        bool Load();
        bool Unload();
        ResourceStatus GetStatus();
        bool IsLoaded();
        bool IsReloading();
        template <typename T>
        Ref<T> GetData();
    };

    // Resource Class
    template <typename T>
    class Resource
    {
    public:
        Ref<T> GetRef();
		const Ref<T> GetRef() const;

        T& operator*();
		const T& operator*() const;

        ResourceStatus GetStatus();
        bool IsLoaded();
		bool IsReloading();

        bool Reload();
        bool Load();
        bool Unload();

		std::filesystem::path GetPath() const;
		ResourceData::LoadFunc GetLoader() const;

    private:
        Resource(ResourceIDType id, Ref<ResourceData> data);

        ResourceIDType m_ID;
        Ref<ResourceData> m_Data;

        friend class ResourceManager;
    };

    // ResourceManager Class
    class ResourceManager
    {
    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        template <typename T>
        Resource<T> GetResource(const std::filesystem::path& path, ResourceData::LoadFunc loader, bool immediateLoad = false);
        template <typename T>
        Resource<T> GetResource(const std::filesystem::path& path, bool immediateLoad = false);
        template <typename T>
        Resource<T> GetResource(ResourceIDType id);

    private:
        RK_AUTO_MUTEX

    	ResourceIDType m_CurrID = 1;
		std::unordered_map<ResourceIDType, Ref<ResourceData>> m_Resources;
        std::unordered_map<std::filesystem::path, uint32_t> m_ResourcePaths;
    };
}

#pragma region definition
namespace Reksi
{
    template<typename T>
    Resource<T> ResourceManager::GetResource(const std::filesystem::path& path, ResourceData::LoadFunc loader, bool immediateLoad)
    {
        ResourceIDType id = 0;

        {
	        RK_AUTO_LOCK_SCOPE

        	auto it = m_ResourcePaths.find(path);
			if (it != m_ResourcePaths.end())
			{
				id = it->second;
			}
        }

		if (id) return GetResource<T>(id);

        // Create new resource
		Ref<ResourceData> data = CreateRef<ResourceData>(ResourceData{ path, nullptr, ResourceStatus::NotLoaded, false, loader });

        {
            RK_AUTO_LOCK_SCOPE

        	id = m_CurrID++;
			// Add to resource paths
            m_ResourcePaths[path] = id;
			// Add to resources
            m_Resources[id] = data;
        }

        if (immediateLoad)
        {
            data->Load();
        }

		return Resource<T>{id, data};
    }

    template<typename T>
    Resource<T> ResourceManager::GetResource(const std::filesystem::path& path, bool immediateLoad)
    {
        ResourceIDType id = 0;

        {
	        RK_AUTO_LOCK_SCOPE

        	auto it = m_ResourcePaths.find(path);
			if (it != m_ResourcePaths.end())
			{
				id = it->second;
			}
        }

		if (id) return GetResource<T>(id);

		// Use the constructor the T type to create a loader
		ResourceData::LoadFunc loader = [](const std::filesystem::path& path) -> Ref<void>
			{
                return CreateRef<T>(path);
			};
		// Create new resource
		Ref<ResourceData> data = CreateRef<ResourceData>(ResourceData{ path, nullptr, ResourceStatus::NotLoaded, false, loader });

        {
			RK_AUTO_LOCK_SCOPE

        	id = m_CurrID++;
			// Add to resource paths
            m_ResourcePaths[path] = id;
			// Add to resources
			m_Resources[id] = data;
        }

		return Resource<T>{id, data};
    }

    template<typename T>
    Resource<T> ResourceManager::GetResource(ResourceIDType id)
    {
        Ref<ResourceData> data;

        {
	        RK_AUTO_LOCK_SCOPE
        	auto it = m_Resources.find(id);
			if (it == m_Resources.end())
			{
				return Resource<T>(0, nullptr);
			}
			data = it->second;
        }

        return Resource<T>{id, data};
    }
}
#pragma endregion
