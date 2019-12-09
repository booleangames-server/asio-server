#pragma once
#include <atomic>
#include <thread>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <boost/pool/pool.hpp>
#include <mutex>
#include "mem_pool.hpp"

namespace io_engine {

const std::size_t BUFFER_SIZE = 64 * 1024 * 2;

thread_local std::shared_ptr<boost::pool<>> __tlsBufferPool{ nullptr };
template<typename _T>
class BufferPool : public boost::serialization::singleton<BufferPool<_T>>
{
public:
	friend class boost::serialization::singleton<BufferPool>;
	static BufferPool& getInstance()
	{
		return boost::serialization::singleton<BufferPool>::get_mutable_instance();
	}

public:
	BufferPool() noexcept
	{		
		__tlsBufferPool = std::make_shared<boost::pool<>>(BUFFER_SIZE);
		_pool[std::this_thread::get_id()] = __tlsBufferPool;
		
	}

	~BufferPool()
	{
		for (auto p : _pool)
		{
			p.second->release_memory();
		}
	}

	_T* aquire()
	{
		void* mem = nullptr;
		if (false == _queue.pop(mem))
		{
			if (__tlsBufferPool == nullptr)
			{
				if (_pool.find(std::this_thread::get_id()) == _pool.end())
				{
					__tlsBufferPool = std::make_shared<boost::pool<>>(BUFFER_SIZE);
					_pool[std::this_thread::get_id()] = __tlsBufferPool;
				}
				__tlsBufferPool = _pool[std::this_thread::get_id()];
			}

			mem = __tlsBufferPool->ordered_malloc();
		}

		if (mem == nullptr) return nullptr;
		auto header = reinterpret_cast<_T*>(mem);
		new(header) _T(BUFFER_SIZE);
		return header;
	}

	void release(_T* b_)
	{
		b_->clear();
		while (!_queue.push(b_))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
	}

private:

	const std::size_t LFQUEQE_SIZE = 65536;
	std::unordered_map<std::thread::id, std::shared_ptr<boost::pool<>>> _pool;
	boost::lockfree::queue<void*> _queue{ LFQUEQE_SIZE };
};

//const std::size_t MEMORY_ALLOCATION_ALIGNMENT = 16;
class buffer
{
public:
	explicit buffer(std::size_t s_) noexcept : _size(s_)
	{
		_ref.store(1);
		auto ptr = this;
		++ptr;

		_base = reinterpret_cast<void*>(ptr);
		_start = _base;

		_start = alignment(_start);
	}

	~buffer() = default;

	void* start() { return _start; }

	std::size_t freeSize()
	{
		return _size - reinterpret_cast<std::size_t>(_start) + reinterpret_cast<std::size_t>(_base) - sizeof(*this);
	}

	void commit(std::size_t s_)
	{
		_start = reinterpret_cast<void*>((reinterpret_cast<std::size_t>(_start) + s_));
		alignment(_start);
	}

	void clear()
	{
		_ref.store(1);
		_start = _base;
		alignment(_start);
	}

	int refCount()
	{
		return _ref.load();
	}
	
	void incRef()
	{
		_ref.fetch_add(1);
	}

	void decRef()
	{
		if (_ref.fetch_sub(1) == 1)
		{
			// buffer memory deallocate
			BufferPool<buffer>::getInstance().release(this);
		}
	}
private:
	void* alignment(void* p_)
	{
		if (reinterpret_cast<size_t>(p_) % MEMORY_ALLOCATION_ALIGNMENT != 0)
		{
			return reinterpret_cast<void*>(
				reinterpret_cast<std::size_t>(p_) + (MEMORY_ALLOCATION_ALIGNMENT - (reinterpret_cast<size_t>(p_) % MEMORY_ALLOCATION_ALIGNMENT)));
		}
		return p_;
	}

private:
	std::size_t _size{ 0 };
	void* _start{ nullptr };
	void* _base{ nullptr };
	std::atomic<int> _ref;
};
thread_local buffer* __tlsbuffer = nullptr;

class BufferHelper
{
public:
	explicit BufferHelper(std::size_t s_) noexcept : _length(s_)
	{
		if (__tlsbuffer == nullptr)
		{
			__tlsbuffer = BufferPool<buffer>::getInstance().aquire();
		}

		if (__tlsbuffer->freeSize() < _length)
		{
			if (__tlsbuffer->refCount() > 1)
			{
				__tlsbuffer = BufferPool<buffer>::getInstance().aquire();
				if (__tlsbuffer == nullptr)
				{
					__tlsbuffer = new buffer{ BUFFER_SIZE };
				}
			}
			else
			{
				__tlsbuffer->clear();
			}
		}

		_buff = __tlsbuffer;
		_buff->incRef();
		_start = _buff->start();
		_buff->commit(_length);
		_pos = _start;
	}

	~BufferHelper()
	{
		_buff->decRef();
	}

	void* start() { return _start; }
	std::size_t length() { return reinterpret_cast<std::size_t>(_pos) - reinterpret_cast<std::size_t>(_start); }

	template<typename _T>
	void write(_T v_)
	{
		memcpy(_pos, &v_, sizeof(v_));
		_pos = reinterpret_cast<void*>(reinterpret_cast<std::size_t>(_pos) + sizeof(v_));
	}

	void write(void* v_, std::size_t l_)
	{
		memcpy(_pos, v_, l_);
		_pos = reinterpret_cast<void*>(reinterpret_cast<std::size_t>(_pos) + l_);
	}

	template<typename _T>
	void read(_T& v_)
	{
		v_ = *(_T*)(_start);
		_start += sizeof(_T);
	}

	void read(void* v_, std::size_t l_)
	{
		memcpy(v_, _start, l_);
		_start = reinterpret_cast<void*>(reinterpret_cast<std::size_t>(_start) + l_);
	}
private:
	buffer* _buff{ nullptr };
	void* _start{ nullptr };
	std::size_t _length{ 0 };
	void* _pos{ nullptr };

};

}