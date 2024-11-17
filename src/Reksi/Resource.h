#pragma once

#include "Reksi/Base.h"
#include "Reksi/ResourceData.h"

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


#pragma region Defer
// Implementation
#include "Reksi/ResourceManager.h"
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
