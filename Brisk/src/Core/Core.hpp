#pragma once

// INCLUDES
#include <memory>
//---------------

namespace Brisk 
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
}

#define DEFINE_BASE_CLASS_CONSTRUCTOR(className) \
protected: \
    className() = default; \
    virtual ~className() = default;

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
//#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }