#ifndef FMI_OOP_MYSTD_UNORDERED_MAP_H
#define FMI_OOP_MYSTD_UNORDERED_MAP_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>

#ifndef FMI_OOP_USE_MYSTD_UNORDERED_MAP

#include <unordered_map>

namespace mystd {

using std::unordered_map;

}

#else
#endif  // FMI_OOP_USE_MYSTD_UNORDERED_MAP

#endif  //FMI_OOP_MYSTD_UNORDERED_MAP_H
