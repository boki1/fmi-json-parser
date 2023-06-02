#ifndef FMI_OOP_MYSTD_OPTIONAL_H
#define FMI_OOP_MYSTD_OPTIONAL_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>

#ifndef FMI_OOP_USE_MYSTD_OPTIONAL

#include <optional>

namespace mystd {
using std::optional;
}

#else

// TODO:
// Implement mystd::optional.

#endif  // FMI_OOP_USE_MYSTD_OPTIONAL

#endif  //FMI_OOP_MYSTD_OPTIONAL_H
