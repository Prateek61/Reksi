#pragma once

#include "Reksi/Base.h"
#include "Reksi/ResourceData.h"

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
