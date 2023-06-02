#ifndef FMI_OOP_MYSTD_UTILITY_H
#define FMI_OOP_MYSTD_UTILITY_H

#include <cassert>

// Always
namespace mystd {

[[noreturn]] inline void unreachable() {
	assert(0);
}

}

#ifndef FMI_OOP_USE_MYSTD_UTILITY
#include <utility>
namespace mystd {
using std::forward;
}
#elif
#endif // FMI_OOP_USE_MYSTD_UTILITY

#endif // FMI_OOP_MYSTD_UTILITY_H
