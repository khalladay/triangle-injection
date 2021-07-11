#pragma once
#include "common_header.h"
#include <vector>
#include <tuple>

typedef uint32_t DelegateHandle;

template<typename RetType, typename ...Args>
class Delegate
{
	using DelegateFuncType = RetType(*)(Args& ...args);
	using BoundDelegateFunc = std::tuple<DelegateHandle, DelegateFuncType>;

	std::vector<BoundDelegateFunc> callbacks;

	DelegateHandle nextHandle = 0;

public:
	DelegateHandle bind(RetType(*func)(Args& ...args))
	{
		callbacks.push_back(BoundDelegateFunc(nextHandle++, func));
		return nextHandle - 1;
	}

	void unbind(DelegateHandle handle)
	{
		for (size_t i = 0; i < callbacks.size(); ++i)
		{
			const auto& cb = callbacks[i];
			if (std::get<0>(cb) == handle)
			{
				callbacks.erase(callbacks.begin() + i);
				break;
			}
		}
	}

	void broadcast(Args& ...args)
	{
		for (const auto& cb : callbacks)
		{
			std::get<1>(cb)(args...);
		}
	}

	bool isBound()
	{
		return callbacks.size() > 0;
	}

};