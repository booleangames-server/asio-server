#pragma once
#include "log.hpp"
#include "fmtlib/format.h"

namespace utility {	namespace Log {

class ConsoleLogWriter: public LogWriter
{

public:
	ConsoleLogWriter() : LogWriter(write_console) {}
	virtual ~ConsoleLogWriter() {}

	virtual void write(LogData&& d_) override
	{
		tm* t = localtime(&d_._time);
		auto dateTime = fmt::format("{0:04}-{1:02}-{2:02} {3:02}:{4:02}:{5:02}",
			t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

		auto log = fmt::format("[{0}][{1}][{2}][{3}({4})] {5} ", dateTime, LogTrivial(d_._trivial), d_._thread, d_._func, d_._line, d_._desc);

		if (false == d_._params.empty())
			d_.dump(log);
			//d_.dump2json(log);

		std::cout << log << std::endl;
	}
};

}}

