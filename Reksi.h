#pragma once

/*
 ____   _         _     __                          ____         _                _    _               
|  _ \ | |  __ _ | |_  / _|  ___   _ __  _ __ ___  |  _ \   ___ | |_   ___   ___ | |_ (_)  ___   _ __  
| |_) || | / _` || __|| |_  / _ \ | '__|| '_ ` _ \ | | | | / _ \| __| / _ \ / __|| __|| | / _ \ | '_ \ 
|  __/ | || (_| || |_ |  _|| (_) || |   | | | | | || |_| ||  __/| |_ |  __/| (__ | |_ | || (_) || | | |
|_|    |_| \__,_| \__||_|   \___/ |_|   |_| |_| |_||____/  \___| \__| \___| \___| \__||_| \___/ |_| |_|
                                                                                                       
*/

#ifndef REKSI_CUSTOM_SP
#define REKSI_CUSTOM_SP 0
#else
#define REKSI_CUSTOM_SP 1
#endif

#ifndef REKSI_THREADING
#define REKSI_THREADING 1
#endif

/*
 * Debug Definition
 */
#if !defined(REKSI_DEBUG) && !defined(REKSI_NDEBUG)
// Set as debug
#define REKSI_DEBUG
#endif

/*
 ____          __  _         _  _    _                    
|  _ \   ___  / _|(_) _ __  (_)| |_ (_)  ___   _ __   ___ 
| | | | / _ \| |_ | || '_ \ | || __|| | / _ \ | '_ \ / __|
| |_| ||  __/|  _|| || | | || || |_ | || (_) || | | |\__ \
|____/  \___||_|  |_||_| |_||_| \__||_| \___/ |_| |_||___/
                                                          
*/


#pragma region Base Macros
// Concat Macro
#define CONCAT(x, y) x#y

/*
 * Definition of Debug Macros
 */
#ifdef REKSI_DEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x)
#endif
#pragma endregion

#pragma region Thread Synchronization Macros
/*
* Definition of Base Thread Synchronization Macros
* Ex. Mutex, Locks, Atomic Operations etc.
*/
#if REKSI_THREADING == 1
// Mutexes
#include <shared_mutex>
#define REKSI_THREADING_MUTABLE mutable
using REKSI_MUTEX_T = std::shared_mutex;
#define REKSI_MUT_IMPL(Mutex) std::shared_mutex Mutex
#define REKSI_LOCK_SHARED_IMPL(Mutex, Lock) std::shared_lock Lock(Mutex)
#define REKSI_LOCK_UNIQUE_IMPL(Mutex, Lock) std::unique_lock Lock(Mutex)
#define REKSI_LOCK_IMPL(Mutex, Lock) std::lock_guard Lock(Mutex)
// Conditional Variables
#include <condition_variable>
using REKSI_CV_T = std::condition_variable_any;
#define REKSI_CV_IMPL(CV) std::condition_variable_any CV
#define REKSI_CV_WAIT_IMPL(CV, Lock, Condition) CV.wait(Lock, Condition)
#define REKSI_CV_NOTIFY_ONE_IMPL(CV) CV.notify_one()
#define REKSI_CV_NOTIFY_ALL_IMPL(CV) CV.notify_all()
#else
#define REKSI_THREADING_MUTABLE
#define REKSI_MUT_IMPL(x)
#define REKSI_SMUT_IMPL(x)
#define REKSI_LOCK_SHARED_IMPL(x, y)
#define REKSI_LOCK_UNIQUE_IMPL(x, y)
#define REKSI_LOCK_IMPL(x, y)
#define REKSI_CV_IMPL(x)
#define REKSI_CV_WAIT_IMPL(x, y, z)
#define REKSI_CV_NOTIFY_ONE_IMPL(x)
#define REKSI_CV_NOTIFY_ALL_IMPL(x)
#endif

/*
* Definition of All Thread Synchronization Macros
*/
#define REKSI_MUTEX(Mutex) REKSI_MUT_IMPL(Mutex)
#define REKSI_LOCK_SHARED(Mutex, Lock) REKSI_LOCK_SHARED_IMPL(Mutex, Lock)
#define REKSI_LOCK_UNIQUE(Mutex, Lock) REKSI_LOCK_UNIQUE_IMPL(Mutex, Lock)
#define REKSI_LOCK(Mutex, Lock) REKSI_LOCK_IMPL(Mutex, Lock)
// Default names
#define REKSI_MUTEX_AUTO_NAME RkAutoMutex
#define REKSI_LOCK_AUTO_NAME RkAutoLock
#define REKSI_MUTEX_AUTO REKSI_MUTEX(REKSI_MUTEX_AUTO_NAME)
#define REKSI_LOCK_SHARED_AUTO REKSI_LOCK_SHARED(REKSI_MUTEX_AUTO_NAME, REKSI_LOCK_AUTO_NAME)
#define REKSI_LOCK_UNIQUE_AUTO REKSI_LOCK_UNIQUE(REKSI_MUTEX_AUTO_NAME, REKSI_LOCK_AUTO_NAME)
#define REKSI_LOCK_AUTO REKSI_LOCK(REKSI_MUTEX_AUTO_NAME, REKSI_LOCK_AUTO_NAME)

#define REKSI_CV(CV) REKSI_CV_IMPL(CV)
#define REKSI_CV_WAIT(CV, Lock, Condition) REKSI_CV_WAIT_IMPL(CV, Lock, Condition)
#define REKSI_CV_NOTIFY_ONE(CV) REKSI_CV_NOTIFY_ONE_IMPL(CV)
#define REKSI_CV_NOTIFY_ALL(CV) REKSI_CV_NOTIFY_ALL_IMPL(CV)
#define REKSI_CV_AUTO_NAME RkAutoCV
#define REKSI_CV_AUTO REKSI_CV(REKSI_CV_AUTO_NAME)
#define REKSI_CV_WAIT_AUTO(Condition) REKSI_CV_WAIT(REKSI_CV_AUTO_NAME, REKSI_LOCK_AUTO_NAME, Condition)
#define REKSI_CV_NOTIFY_ONE_AUTO REKSI_CV_NOTIFY_ONE(REKSI_CV_AUTO_NAME)
#define REKSI_CV_NOTIFY_ALL_AUTO REKSI_CV_NOTIFY_ALL(REKSI_CV_AUTO_NAME)

#pragma endregion

#pragma region Smart Pointer Definitions
#if REKSI_CUSTOM_SP == 0
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

	// Weak Pointer
	template <typename T>
	using WeakPtr = std::weak_ptr<T>;

	// Static Pointer Cast
	template <typename T, typename U>
	constexpr SharedPtr<T> StaticSharedCast(const SharedPtr<U>& ptr)
	{
		return std::static_pointer_cast<T>(ptr);
	}

	// Dynamic Pointer Cast
	template <typename T, typename U>
	constexpr SharedPtr<T> DynamicSharedCast(const SharedPtr<U>& ptr)
	{
		return std::dynamic_pointer_cast<T>(ptr);
	}
}

#endif

#pragma endregion


/*
 ____                    
| __ )   __ _  ___   ___ 
|  _ \  / _` |/ __| / _ \
| |_) || (_| |\__ \|  __/
|____/  \__,_||___/ \___|
                         
*/


// Standard Includes
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include <list>
#include <typeindex>


// Forward Declarations
namespace Reksi
{
	class ResourceManager;
	class ResourceData;
	template <typename T>
	class Resource;
}


/*
 ____                                              ____          _          
|  _ \   ___  ___   ___   _   _  _ __   ___   ___ |  _ \   __ _ | |_   __ _ 
| |_) | / _ \/ __| / _ \ | | | || '__| / __| / _ \| | | | / _` || __| / _` |
|  _ < |  __/\__ \| (_) || |_| || |   | (__ |  __/| |_| || (_| || |_ | (_| |
|_| \_\ \___||___/ \___/  \__,_||_|    \___| \___||____/  \__,_| \__| \__,_|
                                                                            
*/


#define RK_BIT(x) (1 << (x))

namespace Reksi
{
	// Forward Declaration
	class ResourceData;

	using ResourceHandleT = uint32_t;

	class ResourceStatus
	{
	public:
		using StateT = uint32_t;

		enum States : StateT
		{
			Loaded = RK_BIT(0),
			Loading = RK_BIT(1),
			MarkedForReload = RK_BIT(3),
			MarkedForDelete = RK_BIT(4)
		};

		StateT State;

		ResourceStatus()
			: State(0u)
		{
		}

		ResourceStatus& Set(States state);
		bool Is(States state) const;
		ResourceStatus& Clear(States state);
	};

	class ResourceLoadStatus
	{
	public:
		using StateT = uint32_t;

		enum States : StateT
		{
			Success = RK_BIT(0),
			Reloaded = RK_BIT(1),
			WaitedForLoad = RK_BIT(2),
			MarkedForDelete = RK_BIT(3),
			AlreadyReloading = RK_BIT(4),
		};

		ResourceLoadStatus()
			: State(0u)
		{
		}

		StateT State;

		ResourceLoadStatus& Set(States state);
		bool Is(States state) const;
		ResourceLoadStatus& Clear(States state);
	};

	enum class ResourceUnloadStatus
	{
		Success,
		Failure,
		Loading
	};

	class ResourceListener
	{
	public:
		ResourceListener() = default;
		virtual ~ResourceListener() = default;

		virtual void OnLoadComplete(ResourceData& data, ResourceLoadStatus status)
		{
		}

		virtual void OnUnloadComplete(ResourceData& data, ResourceUnloadStatus status)
		{
		}

		virtual void BeforeDeleting(ResourceData& data)
		{
		}
	};

	template <typename T>
	using ResourceLoadFunc = std::function<SharedPtr<T>(const std::filesystem::path&)>;

	class ResourceData
	{
	public:
		using ListenerList = std::list<ResourceListener*>;

		// Public for external use
		REKSI_THREADING_MUTABLE REKSI_MUTEX_AUTO;
		REKSI_CV_AUTO;

		ResourceStatus GetStatus() const;
		bool IsState(ResourceStatus::States state) const;
		ResourceLoadStatus Load();
		ResourceUnloadStatus Unload();
		template <typename T>
		SharedPtr<T> GetData();
		void AddListener(ResourceListener* listener);
		void RemoveListener(ResourceListener* listener);
		void ClearListeners();
		void AddListeners(const ListenerList& listeners);
		void WaitUntilCurrentLoading();
		std::filesystem::path GetPath() const;
		template <typename T>
		ResourceLoadFunc<T> GetLoader() const;
		ResourceHandleT GetHandle() const;
		std::type_index GetTypeIndex() const;

		~ResourceData();

	private:
		using LoadFunc = std::function<SharedPtr<void>(const std::filesystem::path&)>;
		using RLS = ResourceLoadStatus;
		using RS = ResourceStatus;
		using RUS = ResourceUnloadStatus;

		ResourceData(ResourceHandleT handle, std::filesystem::path path, LoadFunc loader,
		             ResourceManager* creator, std::type_index typeIndex);

		ResourceHandleT m_Handle;
		ResourceStatus m_Status;
		std::filesystem::path m_Path;
		LoadFunc m_Loader;
		SharedPtr<void> m_Data;
		ListenerList m_Listeners;
		ResourceManager* m_Creator;
		const std::type_index m_TypeIndex;

		ListenerList GetListenersCopy() const;
		void NotifyListenersOnLoadComplete(ResourceLoadStatus status);
		void NotifyListenersOnUnloadComplete(ResourceUnloadStatus status);
		void NotifyListenersBeforeDeleting();

		void SetState(ResourceStatus::States state);
		void ClearState(ResourceStatus::States state);

		// Thread unsafe, use with caution
		// Performs more checks in Debug mode
		template <typename T>
		SharedPtr<T> GetDataInternal();

		// Just perform load without notifying listeners or the manager
		ResourceLoadStatus LoadInternal();
		// Just perform unload without notifying listeners or the manager
		ResourceUnloadStatus UnloadInternal();

		friend class ResourceManager;
	};
}


/*
 ____                                             
|  _ \   ___  ___   ___   _   _  _ __   ___   ___ 
| |_) | / _ \/ __| / _ \ | | | || '__| / __| / _ \
|  _ < |  __/\__ \| (_) || |_| || |   | (__ |  __/
|_| \_\ \___||___/ \___/  \__,_||_|    \___| \___|
                                                  
*/


namespace Reksi
{
	// Resource class is a wrapper around the ResourceData class, and is returned by the ResourceManager
	template <typename T>
	class Resource
	{
	public:
		SharedPtr<T> GetRef();
		T& operator*();

		ResourceStatus GetStatus() const;
		bool IsLoaded() const;
		bool IsValid() const;

		ResourceLoadStatus Load();
		ResourceUnloadStatus Unload();
		ResourceLoadStatus Reload();

		std::filesystem::path GetPath() const;
		ResourceLoadFunc<T> GetLoader() const;

		ResourceManager* GetManager() const;
		ResourceHandleT GetHandle() const;

		void AddListener(ResourceListener* listener);
		void RemoveListener(ResourceListener* listener);
		void ClearListeners();

	private:
		Resource(ResourceHandleT handle, ResourceData* data, ResourceManager* manager);

		ResourceHandleT m_Handle;
		ResourceData* m_Data;
		ResourceManager* m_Manager;

		friend class ResourceManager;
	};
}


/*
 ____                                              __  __                                         
|  _ \   ___  ___   ___   _   _  _ __   ___   ___ |  \/  |  __ _  _ __    __ _   __ _   ___  _ __ 
| |_) | / _ \/ __| / _ \ | | | || '__| / __| / _ \| |\/| | / _` || '_ \  / _` | / _` | / _ \| '__|
|  _ < |  __/\__ \| (_) || |_| || |   | (__ |  __/| |  | || (_| || | | || (_| || (_| ||  __/| |   
|_| \_\ \___||___/ \___/  \__,_||_|    \___| \___||_|  |_| \__,_||_| |_| \__,_| \__, | \___||_|   
                                                                                |___/             
*/


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


/*
 ____          __  _         _  _    _                    
|  _ \   ___  / _|(_) _ __  (_)| |_ (_)  ___   _ __   ___ 
| | | | / _ \| |_ | || '_ \ | || __|| | / _ \ | '_ \ / __|
| |_| ||  __/|  _|| || | | || || |_ | || (_) || | | |\__ \
|____/  \___||_|  |_||_| |_||_| \__||_| \___/ |_| |_||___/
                                                          
*/

#pragma region Defer
// Implementation
namespace Reksi
{
	inline ResourceData::ResourceData(ResourceHandleT handle, std::filesystem::path path, LoadFunc loader,
	                                  ResourceManager* creator, std::type_index typeIndex)
		: m_Handle(handle),
		  m_Path(std::move(path)),
		  m_Loader(std::move(loader)),
		  m_Creator(creator),
		  m_TypeIndex(typeIndex)
	{
	}

	inline ResourceStatus ResourceData::GetStatus() const
	{
		REKSI_LOCK_SHARED_AUTO;
		return m_Status;
	}

	inline bool ResourceData::IsState(ResourceStatus::States state) const
	{
		REKSI_LOCK_SHARED_AUTO;
		return static_cast<bool>(state & m_Status.State);
	}

	inline ResourceLoadStatus ResourceData::Load()
	{
		// Internal Load
		const RLS status = LoadInternal();
		// Notify listeners
		NotifyListenersOnLoadComplete(status);

		// TODO: Notify the resource manager as well

		// Return status
		return status;
	}

	inline ResourceUnloadStatus ResourceData::Unload()
	{
		// Internal Unload
		const ResourceUnloadStatus status = UnloadInternal();
		// Notify listeners
		NotifyListenersOnUnloadComplete(status);

		// TODO: Notify the resource manager as well

		// Return status
		return status;
	}

	template <typename T>
	SharedPtr<T> ResourceData::GetData()
	{
		{
			// If data is loaded, return it
			REKSI_LOCK_SHARED_AUTO;

			if ( m_Status.Is(ResourceStatus::Loaded) )
			{
				return GetDataInternal<T>();
			}
		}

		auto status = Load();

		{
			// Return the data
			REKSI_LOCK_SHARED_AUTO;

			return GetDataInternal<T>();
		}
	}

	template <typename T>
	SharedPtr<T> ResourceData::GetDataInternal()
	{
		if ( m_TypeIndex != typeid(T) )
		{
			throw std::runtime_error("ResourceData::GetDataInternal: Type mismatch");
		}

		return StaticSharedCast<T>(m_Data);
	}

	inline void ResourceData::AddListener(ResourceListener* listener)
	{
		REKSI_LOCK_UNIQUE_AUTO;

		m_Listeners.emplace_back(listener);
	}

	inline void ResourceData::RemoveListener(ResourceListener* listener)
	{
		REKSI_LOCK_UNIQUE_AUTO;

		m_Listeners.remove(listener);
	}

	inline void ResourceData::ClearListeners()
	{
		REKSI_LOCK_UNIQUE_AUTO;

		m_Listeners.clear();
	}

	inline void ResourceData::AddListeners(const ListenerList& listeners)
	{
		REKSI_LOCK_UNIQUE_AUTO;

		m_Listeners.insert(m_Listeners.end(), listeners.begin(), listeners.end());
	}

	inline void ResourceData::WaitUntilCurrentLoading()
	{
		REKSI_LOCK_UNIQUE_AUTO;

		REKSI_CV_WAIT_AUTO([&] { return !m_Status.Is(ResourceStatus::Loading); });
	}

	inline std::filesystem::path ResourceData::GetPath() const
	{
		REKSI_LOCK_SHARED_AUTO;

		return m_Path;
	}

	template <typename T>
	ResourceLoadFunc<T> ResourceData::GetLoader() const
	{
		REKSI_LOCK_SHARED_AUTO;

		return m_Loader;
	}

	inline ResourceHandleT ResourceData::GetHandle() const
	{
		REKSI_LOCK_SHARED_AUTO;

		return m_Handle;
	}

	inline std::type_index ResourceData::GetTypeIndex() const
	{
		return m_TypeIndex;
	}

	inline ResourceData::~ResourceData()
	{
		NotifyListenersBeforeDeleting();

		REKSI_LOCK_UNIQUE_AUTO;
		m_Status.Set(RS::MarkedForDelete).Clear(RS::MarkedForReload);
		REKSI_CV_WAIT_AUTO([&] { return !m_Status.Is(ResourceStatus::Loading); });
	}

	inline ResourceData::ListenerList ResourceData::GetListenersCopy() const
	{
		ListenerList listeners_copy;

		{
			REKSI_LOCK_SHARED_AUTO;

			// Create a copy of the listeners
			listeners_copy = m_Listeners;
		}

		return listeners_copy;
	}

	inline void ResourceData::NotifyListenersOnLoadComplete(ResourceLoadStatus status)
	{
		for ( const auto listener : GetListenersCopy() )
		{
			listener->OnLoadComplete(*this, status);
		}
	}

	inline void ResourceData::NotifyListenersOnUnloadComplete(ResourceUnloadStatus status)
	{
		for ( const auto listener : GetListenersCopy() )
		{
			listener->OnUnloadComplete(*this, status);
		}
	}


	inline void ResourceData::NotifyListenersBeforeDeleting()
	{
		for ( const auto listener : GetListenersCopy() )
		{
			listener->BeforeDeleting(*this);
		}
	}

	inline void ResourceData::SetState(ResourceStatus::States state)
	{
		REKSI_LOCK_SHARED_AUTO;

		m_Status.Set(state);
	}

	inline void ResourceData::ClearState(ResourceStatus::States state)
	{
		REKSI_LOCK_SHARED_AUTO;

		m_Status.Clear(state);
	}

	/*
	 * If not loaded, goes ahead and loads the resource
	 * If loading on another thread, waits for the load to complete
	 * If already loaded, reloads the resource
	 * If reloading on another thread, returns immediately
	 */
	inline ResourceLoadStatus ResourceData::LoadInternal()
	{
		std::filesystem::path res_path;
		RLS out;

		{
			REKSI_LOCK_UNIQUE_AUTO;

			// Resource is marked for delete
			if ( m_Status.Is(RS::MarkedForDelete) )
			{
				return out.Set(RLS::MarkedForDelete);
			}
			// Resource is previously not loaded and loading, wait for load to complete
			if ( !m_Status.Is(RS::Loaded) && m_Status.Is(RS::Loading) )
			{
				REKSI_CV_WAIT_AUTO([&] { return m_Status.Is(RS::Loaded); });
				return out.Set(RLS::WaitedForLoad);
			}
			// Resource is previously loaded and loading, return already reloading
			if ( m_Status.Is(RS::Loaded) && m_Status.Is(RS::Loading) )
			{
				return out.Set(RLS::AlreadyReloading).Set(RLS::Reloaded);
			}

			// If already loaded
			if ( m_Status.Is(RS::Loaded) )
			{
				out.Set(RLS::Reloaded);
			}
			m_Status.Set(RS::Loading).Clear(RS::MarkedForReload);
			res_path = m_Path;
		}

		// Load the resource
		const auto data = m_Loader(res_path);

		{
			REKSI_LOCK_UNIQUE_AUTO;

			m_Status.Clear(RS::Loading);

			// In case of load failure, clear the loading state
			if ( data )
			{
				m_Data = data;
				m_Status.Set(RS::Loaded);
				out.Set(RLS::Success);
			}
		}

		// Loading is complete, send Condition Variable signal
		REKSI_CV_NOTIFY_ALL_AUTO;

		return out;
	}

	inline ResourceUnloadStatus ResourceData::UnloadInternal()
	{
		REKSI_LOCK_UNIQUE_AUTO;

		m_Data.reset();
		m_Status.Clear(ResourceStatus::Loaded);
		return ResourceUnloadStatus::Success;
	}

	inline ResourceStatus& ResourceStatus::Set(States state)
	{
		State |= state;
		return *this;
	}

	inline bool ResourceStatus::Is(States state) const
	{
		return static_cast<bool>(this->State & state);
	}

	inline ResourceStatus& ResourceStatus::Clear(States state)
	{
		State &= ~state;
		return *this;
	}

	inline ResourceLoadStatus& ResourceLoadStatus::Set(States state)
	{
		State |= state;
		return *this;
	}

	inline bool ResourceLoadStatus::Is(States state) const
	{
		return static_cast<bool>(this->State & state);
	}

	inline ResourceLoadStatus& ResourceLoadStatus::Clear(States state)
	{
		State &= ~state;
		return *this;
	}
}
#pragma endregion


#pragma region Defer
// Implementation
namespace Reksi
{
	template <typename T>
	SharedPtr<T> Resource<T>::GetRef()
	{
		assert(IsValid());

		auto ref = m_Data->GetData<T>();
		// If data is valid, return it, else try to get a default resource from the manager
		if ( ref ) return ref;

		ref = m_Manager->GetDefaultResource<T>();
		return ref;
	}

	template <typename T>
	T& Resource<T>::operator*()
	{
		return *GetRef();
	}

	template <typename T>
	ResourceStatus Resource<T>::GetStatus() const
	{
		assert(IsValid());

		return m_Data->GetStatus();
	}

	template <typename T>
	bool Resource<T>::IsLoaded() const
	{
		assert(IsValid());

		return m_Data->IsState(ResourceStatus::States::Loaded);
	}

	template <typename T>
	bool Resource<T>::IsValid() const
	{
		return this->m_Manager->IsValid(m_Handle);
	}

	template <typename T>
	ResourceLoadStatus Resource<T>::Load()
	{
		assert(IsValid());

		return m_Data->Load();
	}

	template <typename T>
	ResourceUnloadStatus Resource<T>::Unload()
	{
		assert(IsValid());

		return m_Data->Unload();
	}

	template <typename T>
	ResourceLoadStatus Resource<T>::Reload()
	{
		assert(IsValid());

		m_Data->WaitUntilCurrentLoading();
		return m_Data->Load();
	}

	template <typename T>
	std::filesystem::path Resource<T>::GetPath() const
	{
		assert(IsValid());

		return m_Data->GetPath();
	}

	template <typename T>
	ResourceLoadFunc<T> Resource<T>::GetLoader() const
	{
		assert(IsValid());

		return m_Data->GetLoader<T>();
	}

	template <typename T>
	ResourceManager* Resource<T>::GetManager() const
	{
		return m_Manager;
	}

	template <typename T>
	ResourceHandleT Resource<T>::GetHandle() const
	{
		return m_Handle;
	}

	template <typename T>
	void Resource<T>::AddListener(ResourceListener* listener)
	{
		assert(IsValid());

		m_Data->AddListener(listener);
	}

	template <typename T>
	void Resource<T>::RemoveListener(ResourceListener* listener)
	{
		assert(IsValid());

		m_Data->RemoveListener(listener);
	}

	template <typename T>
	void Resource<T>::ClearListeners()
	{
		assert(IsValid());

		m_Data->ClearListeners();
	}

	template <typename T>
	Resource<T>::Resource(ResourceHandleT handle, ResourceData* data, ResourceManager* manager)
		: m_Handle(handle), m_Data(data), m_Manager(manager)
	{
	}

}
#pragma endregion


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
