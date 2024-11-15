#pragma once

#include "Reksi/Base.h"

#define RK_BIT(x) (1 << (x))

namespace Reksi
{
	class ResourceManager;
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

	using ResourceLoadFunc = std::function<SharedPtr<void>(const std::filesystem::path&)>;

	class ResourceData
	{
	public:
		using ListenerList = std::list<ResourceListener*>;

		// Public for external use
		REKSI_MUTEX_AUTO;
		REKSI_CV_AUTO;

		ResourceStatus::StateT GetState() const;
		bool IsState(ResourceStatus::States state) const;
		ResourceLoadStatus Load();
		ResourceUnloadStatus Unload();
		template <typename T>
		SharedPtr<T> GetData();
		void AddListener(ResourceListener* listener);
		void RemoveListener(ResourceListener* listener);
		void ClearListeners();
		void AddListeners(const ListenerList& listeners);
		std::filesystem::path GetPath() const;
		ResourceLoadFunc GetLoader() const;
		ResourceHandleT GetHandle() const;

	private:
		using RLS = ResourceLoadStatus;
		using RS = ResourceStatus;
		using RUS = ResourceUnloadStatus;

		ResourceData(ResourceHandleT handle, std::filesystem::path path, ResourceLoadFunc loader,
		             ResourceManager* creator);

		ResourceHandleT m_Handle;
		ResourceStatus m_Status;
		std::filesystem::path m_Path;
		ResourceLoadFunc m_Loader;
		SharedPtr<void> m_Data;
		ListenerList m_Listeners;
		ResourceManager* m_Creator;

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

// Implementation
namespace Reksi
{
	inline ResourceData::ResourceData(ResourceHandleT handle, std::filesystem::path path, ResourceLoadFunc loader,
	                                  ResourceManager* creator)
		: m_Handle(handle), m_Path(std::move(path)), m_Loader(std::move(loader)), m_Creator(creator)
	{
	}

	inline ResourceStatus::StateT ResourceData::GetState() const
	{
		REKSI_LOCK_SHARED_AUTO;
		return m_Status.State;
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
		DEBUG_ONLY(
			return DynamicSharedCast<T>(m_Data);
		)

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

	inline std::filesystem::path ResourceData::GetPath() const
	{
		REKSI_LOCK_SHARED_AUTO;

		return m_Path;
	}

	inline ResourceLoadFunc ResourceData::GetLoader() const
	{
		REKSI_LOCK_SHARED_AUTO;

		return m_Loader;
	}

	inline ResourceHandleT ResourceData::GetHandle() const
	{
		REKSI_LOCK_SHARED_AUTO;

		return m_Handle;
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
			if (m_Status.Is(RS::MarkedForDelete))
			{
				return out.Set(RLS::MarkedForDelete);
			}
			// Resource is previously not loaded and loading, wait for load to complete
			if (!m_Status.Is(RS::Loaded) && m_Status.Is(RS::Loading))
			{
				REKSI_CV_WAIT_AUTO([&] { return m_Status.Is(RS::Loaded); });
				return out.Set(RLS::WaitedForLoad);
			}
			// Resource is previously loaded and loading, return already reloading
			if (m_Status.Is(RS::Loaded) && m_Status.Is(RS::Loading))
			{
				return out.Set(RLS::AlreadyReloading).Set(RLS::Reloaded);
			}

			// If already loaded
			if (m_Status.Is(RS::Loaded))
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
			if (data)
			{
				m_Data = data;
				m_Status.Set(RS::Loaded);
				out.Set(RLS::Success);
			}
		}

		// Loading is complete, send Condition Variable signal
		if (!out.Is(RLS::Reloaded))
		{
			REKSI_CV_NOTIFY_ALL_AUTO;
		}

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
