#pragma once

#include "Reksi/PlatformDetection.h"

#pragma region Base Macros
// Concat Macro
#define CONCAT(x, y) x#y

/*
 * Definition of Debug Macros
 */
#ifdef REKSI_DEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x)
#endif
#pragma endregion

#pragma region Thread Synchronization Macros
 /*
 * Definition of Base Thread Synchronization Macros
 * Ex. Mutex, Locks, Atomic Operations etc.
 */
#if REKSI_THREADING == 1
// Mutexes
#include <shared_mutex>
#define REKSI_THREADING_MUTABLE mutable
using REKSI_MUTEX_T = std::shared_mutex;
#define REKSI_MUT_IMPL(Mutex) std::shared_mutex Mutex
#define REKSI_LOCK_SHARED_IMPL(Mutex, Lock) std::shared_lock Lock(Mutex)
#define REKSI_LOCK_UNIQUE_IMPL(Mutex, Lock) std::unique_lock Lock(Mutex)
#define REKSI_LOCK_IMPL(Mutex, Lock) std::lock_guard Lock(Mutex)
// Conditional Variables
#include <condition_variable>
using REKSI_CV_T = std::condition_variable_any;
#define REKSI_CV_IMPL(CV) std::condition_variable_any CV
#define REKSI_CV_WAIT_IMPL(CV, Lock, Condition) CV.wait(Lock, Condition)
#define REKSI_CV_NOTIFY_ONE_IMPL(CV) CV.notify_one()
#define REKSI_CV_NOTIFY_ALL_IMPL(CV) CV.notify_all()
#else
#define REKSI_THREADING_MUTABLE
#define REKSI_MUT_IMPL(x)
#define REKSI_SMUT_IMPL(x)
#define REKSI_LOCK_SHARED_IMPL(x, y)
#define REKSI_LOCK_UNIQUE_IMPL(x, y)
#define REKSI_LOCK_IMPL(x, y)
#define REKSI_CV_IMPL(x)
#define REKSI_CV_WAIT_IMPL(x, y, z)
#define REKSI_CV_NOTIFY_ONE_IMPL(x)
#define REKSI_CV_NOTIFY_ALL_IMPL(x)
#endif

 /*
 * Definition of All Thread Synchronization Macros
 */
#define REKSI_MUTEX(Mutex) REKSI_MUT_IMPL(Mutex)
#define REKSI_LOCK_SHARED(Mutex, Lock) REKSI_LOCK_SHARED_IMPL(Mutex, Lock)
#define REKSI_LOCK_UNIQUE(Mutex, Lock) REKSI_LOCK_UNIQUE_IMPL(Mutex, Lock)
#define REKSI_LOCK(Mutex, Lock) REKSI_LOCK_IMPL(Mutex, Lock)
// Default names
#define REKSI_MUTEX_AUTO_NAME RkAutoMutex
#define REKSI_LOCK_AUTO_NAME RkAutoLock
#define REKSI_MUTEX_AUTO REKSI_MUTEX(REKSI_MUTEX_AUTO_NAME)
#define REKSI_LOCK_SHARED_AUTO REKSI_LOCK_SHARED(REKSI_MUTEX_AUTO_NAME, REKSI_LOCK_AUTO_NAME)
#define REKSI_LOCK_UNIQUE_AUTO REKSI_LOCK_UNIQUE(REKSI_MUTEX_AUTO_NAME, REKSI_LOCK_AUTO_NAME)
#define REKSI_LOCK_AUTO REKSI_LOCK(REKSI_MUTEX_AUTO_NAME, REKSI_LOCK_AUTO_NAME)

#define REKSI_CV(CV) REKSI_CV_IMPL(CV)
#define REKSI_CV_WAIT(CV, Lock, Condition) REKSI_CV_WAIT_IMPL(CV, Lock, Condition)
#define REKSI_CV_NOTIFY_ONE(CV) REKSI_CV_NOTIFY_ONE_IMPL(CV)
#define REKSI_CV_NOTIFY_ALL(CV) REKSI_CV_NOTIFY_ALL_IMPL(CV)
#define REKSI_CV_AUTO_NAME RkAutoCV
#define REKSI_CV_AUTO REKSI_CV(REKSI_CV_AUTO_NAME)
#define REKSI_CV_WAIT_AUTO(Condition) REKSI_CV_WAIT(REKSI_CV_AUTO_NAME, REKSI_LOCK_AUTO_NAME, Condition)
#define REKSI_CV_NOTIFY_ONE_AUTO REKSI_CV_NOTIFY_ONE(REKSI_CV_AUTO_NAME)
#define REKSI_CV_NOTIFY_ALL_AUTO REKSI_CV_NOTIFY_ALL(REKSI_CV_AUTO_NAME)

#pragma endregion

#pragma region Smart Pointer Definitions
#if REKSI_CUSTOM_SP == 0
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

	// Weak Pointer
	template <typename T>
	using WeakPtr = std::weak_ptr<T>;

	// Static Pointer Cast
	template <typename T, typename U>
	constexpr SharedPtr<T> StaticSharedCast(const SharedPtr<U>& ptr)
	{
		return std::static_pointer_cast<T>(ptr);
	}

	// Dynamic Pointer Cast
	template <typename T, typename U>
	constexpr SharedPtr<T> DynamicSharedCast(const SharedPtr<U>& ptr)
	{
		return std::dynamic_pointer_cast<T>(ptr);
	}
}

#endif

#pragma endregion
