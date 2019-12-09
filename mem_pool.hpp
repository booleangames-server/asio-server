#pragma once
#include <thread>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <boost/pool/pool.hpp>
#include <boost/lockfree/queue.hpp>
#include <mutex>

namespace io_engine {

class MemHeader
{
public:
	std::size_t _s{ 0 };
	MemHeader(std::size_t s_) noexcept : _s(s_)
	{}
};

template<typename _H = MemHeader>
inline void* allocMemHeader(_H* h_, std::size_t s_)
{
	// call construct
	new (h_)_H{s_};

	// move data address
	return ++h_;
}

template<typename _H = MemHeader>
inline void* freeMemHeader(void* m_)
{
	_H* header = reinterpret_cast<_H*>(m_);

	// move header address
	--header;
	return header;
}

thread_local boost::pool<>* __pool = nullptr;
const std::size_t POOL_SIZE = 64;
const std::size_t CHUNK_SIZE = 1024;
const std::size_t CHUNK_TABLE_SIZE = (POOL_SIZE + 4) * 4;
const std::size_t MEMORY_ALLOCATION_ALIGNMENT = 16;

template<typename _H = MemHeader>
class MemPool : public boost::serialization::singleton<MemPool<_H>>
{
	struct ThreadPool
	{
		const std::size_t LFQUEQE_SIZE = 65536;
		std::unordered_map<std::thread::id, std::shared_ptr<boost::pool<>>> _pool;
		boost::lockfree::queue<void*> _queue{ LFQUEQE_SIZE };
		std::size_t _chunkSize;
	public:
		ThreadPool(std::size_t chunkSize_) noexcept :_chunkSize(chunkSize_)
		{
			_pool[std::this_thread::get_id()] = std::make_shared<boost::pool<>>(_chunkSize);
		}
		~ThreadPool()
		{
			for (auto p : _pool)
			{
				p.second->release_memory();
			}
		}

		void* malloc()
		{
			void* mem = nullptr;
			if (_queue.pop(mem))
			{
				return mem;
			}

			if (_pool.find(std::this_thread::get_id()) == _pool.end())
			{
				_pool[std::this_thread::get_id()] = std::make_shared<boost::pool<>>(_chunkSize);
			}

			return _pool[std::this_thread::get_id()]->ordered_malloc();
		}

		void free(void* mem_)
		{
			while (!_queue.push(mem_))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(0));
			}
		}

	};

public:
	friend class boost::serialization::singleton<MemPool>;
	static MemPool& getInstance()
	{
		return boost::serialization::singleton<MemPool>::get_mutable_instance();
	}

public:
	MemPool() noexcept
	{
		std::size_t chunkSizeIndex = 1;
		for (std::size_t i = 0; i <= POOL_SIZE; ++i)
		{
			std::size_t chunkSize = CHUNK_SIZE * (4 * (i + 4));
			if (chunkSize % MEMORY_ALLOCATION_ALIGNMENT == 0)
			{
				auto pool = std::make_unique<ThreadPool>( chunkSize );
				if (chunkSizeIndex >= CHUNK_TABLE_SIZE)
				{
					break;
				}

				while (chunkSizeIndex < chunkSize / CHUNK_SIZE)
				{
					_chunkTable[chunkSizeIndex++] = pool.get();
				}
				_pool.emplace_back(std::move(pool));
			}
			else
			{
				_pool[i] = nullptr;
			}
		}
		_chunkTable[0] = _chunkTable[1];
	}

	~MemPool()
	{
		_pool.clear();
	}


	void* alloc(size_t size_)
	{
		std::size_t chunkSize = size_ + sizeof(_H);
		std::size_t index = (chunkSize / CHUNK_SIZE);

		_H* header = nullptr;
		if (index > CHUNK_TABLE_SIZE || _chunkTable[index] == nullptr)
		{
			header = static_cast<_H*>(::malloc(chunkSize));
		}

		header = static_cast<_H*>(_chunkTable[index]->malloc());

		if (header == nullptr)
		{
			return nullptr;
		}

		return allocMemHeader(header, chunkSize);
	}

	void free(void* mem_)
	{
		// move header address
		_H* header = (_H*)mem_;
		--header;

		size_t chunkSize = header->_s;
		size_t index = (chunkSize / CHUNK_SIZE);

		// check chunk table
		if (index > CHUNK_SIZE || _chunkTable[index] == nullptr)
		{
			auto header = freeMemHeader(mem_);
			::free(header);
			return;
		}

		_chunkTable[index]->free(header);
	}

private:
	std::array<ThreadPool*, CHUNK_TABLE_SIZE> _chunkTable;
	std::vector<std::unique_ptr<ThreadPool>> _pool;
};

}

#define DECLARE_MEMPOOL_NEW_DELETE public: \
	void* operator new(size_t size_) noexcept \
	{ return io_engine::MemPool<io_engine::MemHeader>::getInstance().alloc(size_); } \
	void operator delete(void* ptr_, size_t size_ ) noexcept \
	{ io_engine::MemPool<io_engine::MemHeader>::getInstance().free(ptr_); }

