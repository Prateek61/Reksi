#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <bitset>
#include <fstream>

#include "../Reksi.h"
#include "TestUtils/Loaders.h"

#define LOG(x) std::cout << #x << ": " << (x) << "\n";
#define LOGV(x) std::cout << (x);
#define LOGI(x) std::cout << #x;

using namespace Reksi;

SharedPtr<std::string> FileLoader(const std::filesystem::path& path)
{
	LOGI(Loading Started\n)

	auto ptr = FileStringLoader(path);
	LOG(*ptr)

	LOGI(Loading Finished\n)

	return ptr;
}

template <typename T>
void file_reloader(Resource<T> resource, std::function<void(Resource<T>& res)> callback = nullptr)
{
	std::filesystem::path path = resource.GetPath();
	//Set the last write time of the file to zero
	std::filesystem::file_time_type last_write;

	while ( true )
	{
		// Get the latest write time of the file
		auto latest_write = last_write_time(path);

		if ( latest_write != last_write )
		{
			last_write = latest_write;

			// Reload the resource
			resource.Reload();

			// Run the callback if it exists
			if ( callback )
			{
				callback(resource);
			}
		}
	}
}

class Listener : public ResourceListener
{
public:
	Listener() = default;

	void OnLoadComplete(ResourceData& data, ResourceLoadStatus status) override
	{
		LOGI(Resource Loaded : \n)
		LOG(status.State)
		LOG(data.GetHandle())
	}

	void OnUnloadComplete(ResourceData& data, ResourceUnloadStatus status) override
	{
		LOGI(Resource Unloaded :)
		LOG(data.GetHandle())
	}

	void BeforeDeleting(ResourceData& data) override
	{
		LOGI(Resource Deleting :)
		LOG(data.GetHandle())
	}
};

int main()
{
	ResourceManager manager("assets/");
	manager.SetDefaultLoader<std::string>(FileLoader);
	manager.SetDefaultResource<std::string>(CreateShared<std::string>("Default String"));

	auto listener = new Listener();

	Resource<std::string> res = manager.GetResource<std::string>("test.txt");

	res.AddListener(listener);

	//Create a thread to reload the file
	std::thread file_reloader_thread([res]()
	{
		std::function<void(Resource<std::string>& r)> callback = [](Resource<std::string>& r)
		{
			LOG(*r)
		};
		file_reloader(res, callback);
	});

	file_reloader_thread.join();
}
