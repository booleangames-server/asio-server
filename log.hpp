#pragma once
#include <boost/serialization/singleton.hpp>
#include <boost/lockfree/queue.hpp>
#include <string>
#include <boost/variant.hpp>
#include "thread_helper.hpp"
#include "log_helper.hpp"
#include "mem_pool.hpp"

namespace utility{ namespace Log {

class LogWriter
{
public:
	enum write_type
	{
		write_console,
		write_file,
		write_none,
	};
public:
	LogWriter(write_type type_) : _type{ type_ } {}
	virtual ~LogWriter() {}
	virtual void write(LogData&& d_) = 0;
	virtual void update() {}
	write_type _type{ write_none };
};

class Logger : public boost::serialization::singleton<Logger>
{
	const static std::size_t LOG_DATA_COUNT = 65538;
	const static std::size_t LOG_WRITER_COUNT = 10;
public:
	friend class boost::serialization::singleton<Logger>;
	static Logger& getInstance()
	{
		return boost::serialization::singleton<Logger>::get_mutable_instance();
	}
public:
	Logger() = default;
	~Logger()
	{
		for (auto writer : _writers)
		{
			if (writer != nullptr)
			{
				delete writer;
			}
		}
	}

	void setFilter()
	{

	}
	void addWriter(LogWriter* writer_)
	{
		_writers[writer_->_type] = writer_;
	}

	template<typename ..._ARGS>
	void write(log_trivial tri_, int line_, const char* func_, const char* file_, const char* desc_, _ARGS&&... args_)
	{
		/// TODO. memory pool use, tsl use
		auto data = new LogData{ line_, utility::thread::getThreadId(), tri_, func_, file_, desc_ };
		AddParam(*data, args_...);

		while (!_queue.push(data))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
	}

	void write(log_trivial tri_, int line_, const char* func_, const char* file_, const char* desc_)
	{
		/// TODO. memory pool use, tsl use
		auto data = new LogData{ line_, utility::thread::getThreadId(), tri_, func_, file_, desc_ };

		while (!_queue.push(data))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
	}

	void start()
	{
		_thread = std::make_shared<std::thread>(
			[log = this]() {
			std::cout << "log worker id: " << utility::thread::getThreadId() << std::endl;
			while (log->_running.load())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				log->work();
			}
		});
	}

	void stop()
	{
		_running.store(false);
		_thread->join();
	}

	void work()
	{
		LogData* data = nullptr;
		while (_queue.pop(data))
		{
			for (auto writer : _writers)
			{
				if (writer != nullptr && data != nullptr)
				{
					writer->write(std::move(*data));
				}
			}
		}
	}

public:
	std::atomic<bool> _running{ true };

private:
	boost::lockfree::queue<LogData*> _queue{ LOG_DATA_COUNT };
	std::array<LogWriter*, LOG_WRITER_COUNT> _writers;
	std::shared_ptr<std::thread> _thread{};

};

}}

#define __FILENAME__ ( strrchr(__FILE__,'\\') == 0 ? __FILE__ : strrchr(__FILE__,'\\') + 1 )
#define LOG(__trivial__, __desc__, ...) utility::Log::Logger::getInstance().write(__trivial__, __LINE__, __FUNCTION__, __FILENAME__, __desc__, ## __VA_ARGS__)
