#pragma once
#include <thread>
#include <atomic>

thread_local uint32_t __thread_id = 0;
namespace utility{ namespace thread {

static std::atomic<uint32_t> thread_id{ 1 };
static std::size_t getThreadId()
{
	if (__thread_id == 0)
	{
		__thread_id = thread_id.fetch_add(1);
	}

	return __thread_id;

	//return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

}}
