#pragma once

#include <chrono>
#include <iostream>
#include <shared_mutex>

class Timer
{
public:
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	bool stopped;

	Timer(std::string name)
		: name(std::move(name)), stopped(false)
	{
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		Stop();
	}

	double Stop()
	{
		auto end = std::chrono::high_resolution_clock::now();
		if ( stopped ) return 0.0;
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		double out = static_cast<double>(duration.count()) / 1000.0;
		// std::cout << name << ": " << out << "us\n";
		stopped = true;
		return out;
	}
};

class TimerThread : public Timer
{
public:
	std::shared_mutex& mutex;

	TimerThread(std::string name, std::shared_mutex& mutex)
		: Timer(std::move(name)), mutex(mutex)
	{
	}

	void Stop()
	{
		auto end = std::chrono::high_resolution_clock::now();
		if (stopped)	return;
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		std::lock_guard<std::shared_mutex> lock(mutex);
		std::cout << name << ": " << static_cast<double>(duration.count()) / 1000.0 << "us\n";
		stopped = true;
	}
};
