#pragma once
#include <boost/variant.hpp>
#include <sstream>
#include "mem_pool.hpp"

namespace utility {	namespace Log {
	enum log_trivial
	{
		trace,
		debug,
		info,
		warning,
		error,
		fatal
	};

	inline static const char* LogTrivial(log_trivial tri_)
	{
		switch (tri_)
		{
		case utility::Log::trace:
			return "trace";
		case utility::Log::debug:
			return "debug";
		case utility::Log::info:
			return "info";
		case utility::Log::warning:
			return "warning";
		case utility::Log::error:
			return "error";
		case utility::Log::fatal:
			return "fatal";
		default:
			return "";
		}
		return "";
	}


	//template <class ..._T> struct overload : _T... { using _T::operator()...; };
	//template <class ..._T> overload(_T...)-> overload<_T...>;

	struct Overload : public boost::static_visitor<> {
		Overload(std::stringstream& o) :out(o) {}

		void operator()(int32_t& v) { out << v; }
		void operator()(int8_t& v) { out << v; }
		void operator()(int16_t& v) { out << v; }
		void operator()(int64_t& v) { out << v; }
		void operator()(std::string& v) { out << v; }
		void operator()(const char* v) { out << v; }

		std::stringstream& out;
	};

	struct OverloadJson : public boost::static_visitor<> {
		OverloadJson(std::stringstream& o) :out(o) {}

		void operator()(int32_t& v) { out << v; }
		void operator()(int8_t& v) { out << v; }
		void operator()(int16_t& v) { out << v; }
		void operator()(int64_t& v) { out << v; }
		void operator()(std::string& v) { out << "\"" << v << "\""; }
		void operator()(const char* v) { out << "\"" << v << "\""; }

		std::stringstream& out;
	};

	class LogData
	{
		const static std::size_t RESERVED_PARAMETER_COUNT = 6;
	public:
		typedef boost::variant<int32_t, int8_t, int16_t, int64_t, std::string, const char* > log_variant;

		void dump(std::string& o)
		{
			std::stringstream out;
			out << "{";
			auto ov = Overload{ out };
			for (auto p : _params)
			{
				boost::apply_visitor(ov, p.first);
				out << ":";
				boost::apply_visitor(ov, p.second);
				out << " ";
			}
			out << "}";
			o += out.str();
		}

		void dump2json(std::string& o)
		{
			std::stringstream out;
			out << "\"param\":{";
			auto ov = OverloadJson{ out };
			for (auto p = _params.begin(); p < _params.end(); )
			{
				boost::apply_visitor(ov, p->first);
				out << ":";
				boost::apply_visitor(ov, p->second);
				++p;
				if (p != _params.end()) out << ",";
			}
			out << "}";
			o += out.str();
		}
	public:
		std::size_t _thread{ 0 };
		int _line{ 0 };
		log_trivial _trivial{ info };
		time_t _time{ 0 };
		std::string _func{ "" };
		std::string _file{ "" };
		std::string _desc{ "" };
		// key(string), value
		std::vector<std::pair<log_variant, log_variant>> _params;


	public:
		LogData(int line_, std::size_t thread_, log_trivial tri_, const char* func_, const char* file_, const char* desc_) :
			_thread{ thread_ }, _line{ line_ }, _trivial{ tri_ }, _func{ func_ }, _file{ file_ }, _desc{ desc_ }
		{
			time(&_time);
			_params.reserve(RESERVED_PARAMETER_COUNT);
		}
	DECLARE_MEMPOOL_NEW_DELETE;
	};

	template <typename _T>
	struct TypeFormat
	{
		static const char* GetType()
		{
			return typeid(_T).name();
		}

		//static const std::string ToString(const char* v_)
		//{
		//	return std::string(v_);
		//}
		//static const std::string ToString(const std::string& v_)
		//{
		//	return v_;
		//}
		//static const std::string ToString(const char v_)
		//{
		//	return std::to_string(v_);
		//}
		//static const std::string ToString(const int& v_)
		//{
		//	return std::to_string(v_);
		//}
		//static const std::string ToString(const long& v_)
		//{
		//	return std::to_string(v_);
		//}
		//static const std::string ToString(const double& v_)
		//{
		//	return std::to_string(v_);
		//}
		//static const std::string ToString(const float& v_)
		//{
		//	return std::to_string(v_);
		//}
		template<typename std::enable_if_t<!std::is_base_of<std::string, std::decay_t<_T>>::value>::type>
		static const std::string ToString(_T&& v_)
		{
			return std::to_string(v_);
		}

		template<typename std::enable_if_t<std::is_base_of<std::string, std::decay_t<_T>>::value>::type>
		static const std::string ToString(_T&& v_)
		{
			return v_;
		}

		template<typename std::enable_if_t<std::is_same<char*, std::decay_t<_T>>::value>::type>
		static const std::string ToString(_T&& v_)
		{
			return std::string(v_);
		}
	};


	template <typename _T, typename _V, typename ... _ARGS>
	void AddParam(LogData& data_, const _T& t_, const _V& v_, _ARGS&&... args_)
	{
		data_._params.emplace_back(std::make_pair(LogData::log_variant{ t_ }, LogData::log_variant{ v_ }));
		AddParam(data_, args_...);
	}

	template <typename _T, typename _V>
	void AddParam(LogData& data_, const _T& t_, const _V& v_)
	{
		data_._params.emplace_back(std::make_pair(LogData::log_variant{ t_ }, LogData::log_variant{ v_ }));
	}

}}