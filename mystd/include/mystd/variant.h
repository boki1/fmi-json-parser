#ifndef FMI_OOP_MYSTD_VARIANT_H
#define FMI_OOP_MYSTD_VARIANT_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>

#ifndef FMI_OOP_USE_MYSTD_VARIANT

#include <variant>

namespace mystd {
using std::variant;
}

#else

// TODO:
// Implement mystd::variant.

#endif  // FMI_OOP_USE_MYSTD_VARIANT

#endif  //FMI_OOP_MYSTD_VARIANT_H
