#pragma once

// Flag to enable or disable custom smart pointer implementations, uses std::unique_ptr and std::shared_ptr by default
#ifndef RK_CUSTOM_SP_IMPLEMENTATION
#define RK_CUSTOM_SP_IMPLEMENTATION 0
#endif

// Flag to enable or disable custom mutex implementations, uses std::mutex by default
#ifndef RK_CUSTOM_MUTEX_IMPLEMENTATION
#define RK_CUSTOM_MUTEX_IMPLEMENTATION 0
#endif

// Thread Safety
#ifndef RK_THREAD_SAFE
#define RK_THREAD_SAFE 1
#endif

#if RK_CUSTOM_MUTEX_IMPLEMENTATION == 0
#if RK_THREAD_SAFE
#include <mutex>
#define RK_AUTO_MUTEX std::mutex Mutex;
#define RK_AUTO_LOCK_SCOPE std::lock_guard<std::mutex> lock(Mutex);
#define RK_MUTEX(x) std::mutex x;
#define RK_LOCK_SCOPE(x) std::lock_guard<std::mutex> lock(x);
#else
#define RK_AUTO_MUTEX
#define RK_AUTO_LOCK_SCOPE
#define RK_MUTEX(x)
#define RK_LOCK_SCOPE(x)
#endif
#endif

#if RK_CUSTOM_SP_IMPLEMENTATION == 0
#include <memory>

namespace Reksi
{
	// Unique Pointer
	template <typename T>
	using UniquePtr = std::unique_ptr<T>;

	template <typename T, typename... Args>
	constexpr UniquePtr<T> CreateUnique(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	// Shared Pointer
	template <typename T>
	using SharedPtr = std::shared_ptr<T>;

	template <typename T, typename... Args>
	constexpr SharedPtr<T> CreateShared(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// Static Pointer Cast
	template <typename T, typename U>
	constexpr SharedPtr<T> StaticSharedCast(const SharedPtr<U>& ptr)
	{
		return std::static_pointer_cast<T>(ptr);
	}
}

#endif

// If Mutex macros are not defined, error
#if !defined(RK_AUTO_MUTEX) || !defined(RK_AUTO_LOCK_SCOPE) || !defined(RK_MUTEX) || !defined(RK_LOCK_SCOPE)
#error "One or more Mutex macros are not defined, please define them in Base.h"
#endif

// TODO: If smart pointers are not defined, error

// Include Files
#include <cstdint>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <functional>
