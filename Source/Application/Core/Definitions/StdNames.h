#pragma once

#include <string>
using String = std::string;
using WString = std::wstring;

#include <string_view>
using StringView = std::string_view;

#include <queue>
template<typename T>
using Queue = std::queue<T>;

#include <span>
template<typename T>
using Span = std::span<T>;

#include <vector>
template<typename T>
using Vector = std::vector<T>;

#include <map>
template<typename K, typename V>
using Map = std::map<K, V>;

template<typename K, typename V>
using Pair = std::pair<K, V>;

#include <list>
template<typename T>
using List = std::list<T>;

#include <set>
template<typename T>
using Set = std::set<T>;

#include <filesystem>
namespace FileSystem = std::filesystem;

#include <unordered_map>
template<typename K, typename V, typename...Args>
using HashMap = std::unordered_map<K, V, Args...>;

template<typename T>
using Hash = std::hash<T>;

#include <variant>
template<typename... Args>
using Variant = std::variant<Args...>;

#include <memory>
template<typename T, typename... Args>
using UniquePtr = std::unique_ptr<T, Args...>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;

#include <initializer_list>
template<typename T>
using InitList = std::initializer_list<T>;

template<typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#include <thread>
using Thread = std::thread;
namespace CurrentThread = std::this_thread;

#include <mutex>
using Mutex = std::mutex;
using CondVar = std::condition_variable;

template<typename T>
using LockGuard = std::lock_guard<T>;

#include <shared_mutex>
using SharedMutex = std::shared_mutex;

template<typename T>
using SharedLock = std::shared_lock<T>;

template<typename T>
using UniqueLock = std::unique_lock<T>;

#include <atomic>
template<typename T>
using Atomic = std::atomic<T>;

#include <functional>
template<typename T>
using function = std::function<T>;
using voidPtrFunc = std::function<void*>;
using voidFunc = std::function<void()>;

#include <typeindex>
using TypeIndex = std::type_index;

#include <fstream>
using StringStream = std::stringstream;
using IfStream = std::ifstream;
using OfStream = std::ofstream;
using FileStream = std::fstream;

#include <stdexcept>

template<typename T>
using Optional = std::optional<T>;