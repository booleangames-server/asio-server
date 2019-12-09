#pragma once
#include <experimental/tuple>

namespace io_engine {

template<typename _T, typename ..._ARGS>
class GoodJob
{
public:
	_T _func;
	std::tuple<_ARGS...> _args;

	GoodJob(_T&& f_, _ARGS&& ...args_) noexcept
		: _func(std::forward<_T>(f_)),
		_args(std::make_tuple(std::forward<_ARGS>(args_)...)) {}

	virtual void operator() ()
	{
		std::experimental::apply(_func, _args);
	}
};

template<typename _T, typename ..._ARGS>
inline decltype(auto) makeGoobJob(_T&& lamda_, _ARGS&& ...args_)
{
	return GoodJob<_T, _ARGS...>(std::forward<_T>(lamda_), std::forward<_ARGS>(args_)...);
}

}