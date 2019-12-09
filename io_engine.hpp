#pragma once
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <experimental/tuple>
#include "thread_helper.hpp"

namespace io_engine {
using namespace boost::asio;

class IOEngine
{
public:
	explicit IOEngine(int workerCount_ = 0) noexcept : _workerCount{ workerCount_ }
	{
		if (workerCount_ == 0)
		{
			// defaul setting(cpu count * 2)
			_workerCount = std::thread::hardware_concurrency() * 2;
		}
	}

	virtual ~IOEngine()
	{
		stop();
		_io.reset();
	}

	void stop()
	{
		_worker.release();
		_io.stop();
		_pool.join_all();
		_io.reset();
	}

	void start()
	{
		_worker = std::make_unique<io_service::work>(_io);
		for (int i = 0; i < _workerCount; ++i)
		{
			_pool.create_thread([&io = getIO()]() 
				{ 					
					std::cout << "run worker id: "<< utility::thread::getThreadId() << std::endl;
					io.run();
				}
			);
		}
	}

	io_service& getIO()
	{
		return _io;
	}

	template<typename _T, typename ..._ARGS>
	void asyncJob(_T&& handler_, _ARGS&& ...args_)
	{
		static_assert(!std::is_function<_T>::value, "not a function handler" );
		_io.post(
			[h{ std::forward<_T>(handler_) }, args{ std::make_tuple(std::forward<_ARGS>(args_)...) }]() mutable 
			{ 
				std::experimental::apply(h, args);
			}
		);
	}

	template<typename _T, typename ..._ARGS>
	void syncJob(_T&& handler_, _ARGS&& ...args_)
	{
		static_assert(!std::is_function<_T>::value, "not a function handler");
		_io.post(
			_forJob.wrap(
				[h{ std::forward<_T>(handler_) }, args{ std::make_tuple(std::forward<_ARGS>(args_)...) }]() mutable
				{
					std::experimental::apply(h, args);
				}
			)
		);
	}

	template <typename _T, typename ..._ARGS>
	void asyncTimer(int64_t time_, _T&& handler_, _ARGS&& ...args_)
	{
		static_assert(!std::is_function<_T>::value, "not a function handler");

		auto t = std::make_shared<boost::asio::deadline_timer>(getIO());
		t->expires_from_now(boost::posix_time::milliseconds(time_));
		t->async_wait(
			_forTimerJob.wrap(
				[t, h{ std::forward<_T>(handler_) }, args{ std::make_tuple(std::forward<_ARGS>(args_)...) }](const boost::system::error_code& err_) mutable
				{
					if (!err_)
					{
						std::experimental::apply(h, args);
					}
				}
			)
		);
	}

private:
	int _workerCount{ 0 };
	io_service _io{};
	boost::thread_group _pool{};
	std::unique_ptr<io_service::work> _worker{nullptr};
	io_service::strand _forJob{_io};
	io_service::strand _forTimerJob{ _io };
};

}
