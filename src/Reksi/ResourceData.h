#pragma once

#include "Reksi/Base.h"

// TODO: Use conditional variable

namespace Reksi
{
	class ResourceManager;
	class ResourceData;

	class ResourceStatus
	{
	public:
		using StateT = uint32_t;

		enum States : StateT
		{
			Loaded = 1,
			Loading = 2,
			Reloading = 4,
			MarkedForUnload = 8,
			Unloading = 16,
			MarkedForReload = 32,
			MarkedForDelete = 64
		};
		StateT State;

		ResourceStatus() : State(0u) {}
		ResourceStatus& SetState(States state);
		bool IsState(States state) const;
		void ClearState(States state);
	};
	enum class ResourceLoadStatus
	{
		Success,
		Failure
	};
	enum class ResourceReloadStatus
	{
		Success,
		Failure,
		AlreadyLoading,
		AlreadyReloading,
		AlreadyUnloading
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

		virtual void OnLoadComplete(ResourceData& data, ResourceLoadStatus status) {}
		virtual void OnUnloadComplete(ResourceData& data, ResourceUnloadStatus status) {}
		virtual void OnReloadComplete(ResourceData& data, ResourceReloadStatus status) {}
		virtual void BeforeDeleting(ResourceData& data) {}
	};

	using ResourceLoadFunc = std::function<SharedPtr<void>(const std::filesystem::path&)>;

	class ResourceData
	{
	public:
		using ListenerList = std::list<ResourceListener*>;

		ResourceStatus::StateT GetState();
		bool IsState(ResourceStatus::States state);
		ResourceLoadStatus Load();
		ResourceUnloadStatus Unload();
		ResourceReloadStatus Reload();
		template <typename T>
		SharedPtr<T> GetData();
		void AddListener(ResourceListener* listener);
		void RemoveListener(ResourceListener* listener);
		std::filesystem::path GetPath() const;
		ResourceLoadFunc GetLoader() const;
		
	private:
		ResourceStatus m_Status;
		std::filesystem::path m_Path;
		ResourceLoadFunc m_Loader;
		SharedPtr<void> m_Data;
		ListenerList m_Listeners;
		ResourceManager* m_Creator;

		REKSI_AUTO_MUTEX

	private:
		void ListenersOnLoadComplete(ResourceData* data, ResourceLoadStatus status);
		void ListenersOnUnloadComplete(ResourceData* data, ResourceUnloadStatus status);
		void ListenersOnReloadComplete(ResourceData* data, ResourceReloadStatus status);
		void ListenersBeforeDeleting(ResourceData* data);

		// Just perform load without notifying listeners
		ResourceLoadStatus LoadInternal();
		// Just perform unload without notifying listeners
		ResourceUnloadStatus UnloadInternal();
		// Just perform reload without notifying listeners
		ResourceReloadStatus ReloadInternal();

		friend class ResourceManager;
	};
}