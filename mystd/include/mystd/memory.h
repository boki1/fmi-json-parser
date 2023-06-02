#ifndef FMI_OOP_MYSTD_MEMORY_H
#define FMI_OOP_MYSTD_MEMORY_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>

#ifndef FMI_OOP_USE_MYSTD_MEMORY

#include <memory>

namespace mystd {

using std::unique_ptr;
using std::make_unique;

}

#else
#endif  // FMI_OOP_USE_MYSTD_MEMORY

#endif  //FMI_OOP_MYSTD_MEMORY_H
