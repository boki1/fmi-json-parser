#ifndef FMI_OOP_MYSTD_ALGORITHM_H
#define FMI_OOP_MYSTD_ALGORITHM_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>

#ifndef FMI_OOP_USE_MYSTD_ALGORITHM

#include <algorithm>

namespace mystd {

using std::find;
using std::find_if;

}

#else

#include <cmath> // for std::size_t

namespace mystd {

///
/// Pretty much everything that the implementation requires is
/// provided in the detailed description in cppreference.
///

template <class InputIt, class T>
InputIt find(InputIt first, InputIt last, const T &value) {
  for (; first != last; ++first)
    if (*first == value)
      return first;

  return last;
}

template <class InputIt, class UnaryPredicate>
InputIt find_if(InputIt first, InputIt last, UnaryPredicate p) {
  for (; first != last; ++first)
    if (p(*first))
      return first;

  return last;
}

template <class InputIt, class T>
std::size_t count(InputIt first, InputIt last, const T &value) {
  std::size_t ret = 0;
  for (; first != last; ++first)
    if (*first == value)
      ++ret;
  return ret;
}

template <class InputIt, class UnaryPredicate>
std::size_t count_if(InputIt first, InputIt last, UnaryPredicate p) {
  std::size_t ret = 0;
  for (; first != last; ++first)
    if (p(*first))
      ++ret;
  return ret;
}

}

#endif // FMI_OOP_USE_MYSTD_ALGORITHM

#endif // FMI_OOP_MYSTD_ALGORITHM_H
