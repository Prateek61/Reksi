#pragma once

#include "Reksi/Base.h"

#include <functional>
#include <filesystem>

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

			if (m_Status == ResourceStatus::NotLoaded) m_Status = ResourceStatus::Loading;
			if (m_Status == ResourceStatus::Loading) is_loading = true;
			if (m_Status == ResourceStatus::Loaded) return;
		}

		if (is_loading)
		{
			// Wait for the resource to finish loading
            while (true)
            {
	            {
					RK_AUTO_LOCK_SCOPE

					if (m_Status == ResourceStatus::Loaded) return;
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
