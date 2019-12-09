#pragma once
#include "log.hpp"
#include "fmtlib/format.h"
#include <fstream>
#include <boost/filesystem.hpp>

namespace utility {
	namespace Log {
		const std::size_t _5MB = 1048576 * 5;
		class FileLogWriter : public LogWriter
		{

		public:
			FileLogWriter(const char* name_, const char* path_, std::size_t size_ = _5MB)
				: LogWriter(write_file), _maxFileSize{ size_ }, _curFileSize{ 0 }, _fileNo{ 1 }, _name{name_}, _path{path_}
			{
				createFile();
			}
			virtual ~FileLogWriter() {}

			virtual void write(LogData&& d_) override
			{
				tm* t = localtime(&d_._time);
				auto dateTime = fmt::format("{0:04}-{1:02}-{2:02} {3:02}:{4:02}:{5:02}",
					t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

				auto log = fmt::format("[{0}][{1}][{2}][{3}({4})] {5} ", dateTime, LogTrivial(d_._trivial), d_._thread, d_._func, d_._line, d_._desc);

				if (false == d_._params.empty())
					d_.dump(log);
				//d_.dump2json(log);

				_curFileSize += log.size();
				_file << log << std::endl;
				_file.flush();
			}

			virtual void update() override
			{
			}

		private:

			void checkFileSize()
			{
				if (_curFileSize > _maxFileSize)
				{
					_file.flush();
					_file.close();

					createFile();
				}
			}

			bool createFile()
			{
				//boost::filesystem::path p(boost::filesystem::current_path());
				boost::filesystem::path p(_path.c_str());
				if (false == boost::filesystem::is_directory(boost::filesystem::status(p)))
				{
					if (false == boost::filesystem::create_directory(p))
					{
						return false;
					}
				}

				std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				tm* t = localtime(&now);
				auto dateTime = fmt::format("{0:02}{1:02}-{2:02}{3:02}",
					t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);

				auto filename = fmt::format("{0}/{1}({2})({3}){4}", _path, _name, dateTime, _fileNo++, ".txt");
				_file.open(filename);
				// check failbit streams error state
				if (_file.fail() == true)
				{
					return false;
				}
				return true;
			}

		private:
			// 최대 파일 크기
			size_t _maxFileSize;
			size_t _curFileSize;

			short _fileNo;
			std::string _name;
			std::string _path;
			std::ofstream _file;
		};

	}
}

