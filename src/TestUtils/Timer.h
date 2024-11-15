#pragma once

#include <chrono>
#include <iostream>
#include <mutex>

class Timer
{
public:
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	bool stopped;

	Timer(const std::string& name)
		: name(name), start(std::chrono::high_resolution_clock::now()), stopped(false)
	{
	}

	~Timer()
	{
		Stop();
	}

	void Stop()
	{
		auto end = std::chrono::high_resolution_clock::now();
		if ( stopped ) return;
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << name << ": " << duration.count() << "ms\n";
		stopped = true;
	}
};

class TimerThread : public Timer
{
public:
	std::mutex& mutex;

	TimerThread(const std::string& name, std::mutex& mutex)
		: Timer(name), mutex(mutex)
	{
	}

	void Stop()
	{
		std::lock_guard<std::mutex> lock(mutex);
		Timer::Stop();
	}
};
