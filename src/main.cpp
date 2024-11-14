#include "../Reksi.h"
#include <iostream>
#include <thread>
#include <future>

#define LOG(x) std::cout << #x << ": " << (x) << "\n";
#define LOGI(x) std::cout << #x << "\n";
#define LOGII(x) std::cout << #x;

class ExampleResource
{
public:
	ExampleResource(const std::filesystem::path& path)
	{
		LOGI(Loading Started)
			std::this_thread::sleep_for(std::chrono::seconds(2));
		LOGI(Finished Loading)
	}
};

int main()
{
	Reksi::ResourceManager manager;
	auto resource = manager.GetResource<ExampleResource>("example.txt", false);

	// Call Load asynchronously
	auto future = std::async(std::launch::async, [&] { resource.Load(); });

	while ( !resource.IsLoaded() )
	{
		std::this_thread::yield();
		LOGI(Waiting for resource to load)
	}

	future.wait();
	resource.Reload();
}

