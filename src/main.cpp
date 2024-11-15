#include <iostream>
#include <thread>
#include <future>

#include "Reksi/Base.h"
#include "TestUtils/Timer.h"

#define LOG(x) std::cout << #x << ": " << (x) << "\n";
#define LOGI(x) std::cout << #x << "\n";
#define LOGII(x) std::cout << #x;

REKSI_MUTEX_S_AUTO
REKSI_MUTEX_S(Cout)

void func()
{
	REKSI_LOCK_SHARED_AUTO

	for (int i = 0; i < 100; i++)
	{
		// Sleep this thread for 1 second
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		REKSI_LOCK(Cout, CoutLock)
		LOG(i)
	}
}

int main()
{
	{
		Timer t("Main");

		std::thread t1(func);
		std::thread t2(func);

		t1.join();
		t2.join();
	}

	return 0;
}
