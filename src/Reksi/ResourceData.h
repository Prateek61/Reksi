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
			Reloading = RK_BIT(2),
			MarkedForReload = RK_BIT(3),
			MarkedForDelete = RK_BIT(4)
		};

		StateT State;

		ResourceStatus()
			: State(0u)
		{
		}

		ResourceStatus& SetState(States state);
		bool IsState(States state) const;
		ResourceStatus& ClearState(States state);
	};

	enum class ResourceLoadStatus
	{
		Success,
		Failure,
		WaitedForLoad,
		AlreadyLoaded,
		MarkedForDelete
	};

	enum class ResourceReloadStatus
	{
		Success,
		Failure,
		AlreadyLoading,
		AlreadyReloading,
		MarkedForDelete,
		CalledLoadInstead
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

		virtual void OnReloadComplete(ResourceData& data, ResourceReloadStatus status)
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
		ResourceReloadStatus Reload();
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
		void NotifyListenersOnReloadComplete(ResourceReloadStatus status);
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
		// Just perform reload without notifying listeners or the manager
		ResourceReloadStatus ReloadInternal();

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
		const ResourceLoadStatus status = LoadInternal();
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

	inline ResourceReloadStatus ResourceData::Reload()
	{
		// Internal Reload
		const ResourceReloadStatus status = ReloadInternal();
		// Notify listeners
		NotifyListenersOnReloadComplete(status);

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

			if ( m_Status.IsState(ResourceStatus::Loaded) )
			{
				return GetDataInternal<T>();
			}
		}

		auto status = Load();
		if ( status == ResourceLoadStatus::Failure )
		{
			// TODO: Log error and return default data, maybe from the resource manager if it is available
			return nullptr;
		}

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

	inline void ResourceData::NotifyListenersOnReloadComplete(ResourceReloadStatus status)
	{
		for ( const auto listener : GetListenersCopy() )
		{
			listener->OnReloadComplete(*this, status);
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

		m_Status.SetState(state);
	}

	inline void ResourceData::ClearState(ResourceStatus::States state)
	{
		REKSI_LOCK_SHARED_AUTO;

		m_Status.ClearState(state);
	}

	inline ResourceLoadStatus ResourceData::LoadInternal()
	{
		std::filesystem::path res_path;

		{
			REKSI_LOCK_SHARED_AUTO;

			// Resource is marked for delete
			if (m_Status.IsState(ResourceStatus::MarkedForDelete))
			{
				return ResourceLoadStatus::MarkedForDelete;
			}
			// Resource is already loaded
			if (m_Status.IsState(ResourceStatus::Loaded))
			{
				return ResourceLoadStatus::AlreadyLoaded;
			}
			// Resource is loading
			if (m_Status.IsState(ResourceStatus::Loading))
			{
				// Wait for load to complete and return
				REKSI_CV_WAIT_AUTO([&] { return !m_Status.IsState(ResourceStatus::Loading); });
				return ResourceLoadStatus::WaitedForLoad;
			}
			// Resource is not loaded
			res_path = m_Path;
			m_Status.SetState(ResourceStatus::Loading);
		}

		// Load the resource outside the lock
		auto data = m_Loader(res_path);

		{
			REKSI_LOCK_UNIQUE_AUTO;

			// In case of loading failure, clear the loading state
			if (!data)
			{
				m_Status.ClearState(ResourceStatus::Loading);
				return ResourceLoadStatus::Failure;
			}

			m_Data = data;
			m_Status.ClearState(ResourceStatus::Loading).SetState(ResourceStatus::Loaded);
			return ResourceLoadStatus::Success;
		}
	}

	inline ResourceUnloadStatus ResourceData::UnloadInternal()
	{
		REKSI_LOCK_UNIQUE_AUTO;

		m_Data.reset();
		m_Status.ClearState(ResourceStatus::Loaded);
		return ResourceUnloadStatus::Success;
	}

	inline ResourceReloadStatus ResourceData::ReloadInternal()
	{
		std::filesystem::path file_path;
		bool call_load = false;

		{
			REKSI_LOCK_UNIQUE_AUTO;

			if (m_Status.IsState(ResourceStatus::MarkedForDelete))
			{
				m_Status.ClearState(ResourceStatus::MarkedForReload);
				return ResourceReloadStatus::MarkedForDelete;
			}
			if (m_Status.IsState(ResourceStatus::Reloading))
			{
				return ResourceReloadStatus::AlreadyReloading;
			}
			if (m_Status.IsState(ResourceStatus::Loading))
			{
				return ResourceReloadStatus::AlreadyLoading;
			}
			// Resource is not currently loaded, so call load instead
			if (!m_Status.IsState(ResourceStatus::Loaded))
			{
				call_load = true;
			}
			else
			{
				file_path = m_Path;
			}
			m_Status.ClearState(ResourceStatus::MarkedForReload).SetState(ResourceStatus::Reloading);
		}

		if (call_load)
		{
			auto load_status = Load();

			{
				REKSI_LOCK_UNIQUE_AUTO;
				m_Status.ClearState(ResourceStatus::Reloading);
			}

			return ResourceReloadStatus::CalledLoadInstead;
		}

		// Load the new data
		auto new_data = m_Loader(file_path);

		{
			REKSI_LOCK_UNIQUE_AUTO;

			m_Status.ClearState(ResourceStatus::Reloading);

			// If loaded data is not valid, don't reset previous data
			if (!new_data)
			{
				return ResourceReloadStatus::Failure;
			}

			m_Data = new_data;
			return ResourceReloadStatus::Success;
		}
	}

	inline ResourceStatus& ResourceStatus::SetState(States state)
	{
		State |= state;
		return *this;
	}

	inline bool ResourceStatus::IsState(States state) const
	{
		return static_cast<bool>(this->State & state);
	}

	inline ResourceStatus& ResourceStatus::ClearState(States state)
	{
		State &= ~state;
		return *this;
	}

}
