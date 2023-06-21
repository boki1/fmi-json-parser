#ifndef FMI_OOP_MYSTD_TYPE_TRAITS_H
#define FMI_OOP_MYSTD_TYPE_TRAITS_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>

#ifndef FMI_OOP_USE_MYSTD_TYPE_TRAITS

#include <memory>

namespace mystd {

using std::enable_if;

}

#else

template <bool, typename T = void>
struct enable_if { };

template <typename T>
struct enable_if<true, T> {
	typedef T type;
};

#endif // FMI_OOP_USE_MYSTD_TYPE_TRAITS

#endif // FMI_OOP_MYSTD_TYPE_TRAITS_H
