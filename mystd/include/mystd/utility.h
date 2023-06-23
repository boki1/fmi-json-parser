#ifndef FMI_OOP_MYSTD_UTILITY_H
#define FMI_OOP_MYSTD_UTILITY_H

#include <cassert>
#include <cstdint>
#include <utility> // for std::move and std::forward

// Always
namespace mystd {

[[noreturn]] inline void unreachable() {
	assert(0);
}

}

#ifdef FMI_OOP_USE_MYSTD_UTILITY
#include <utility>
namespace mystd {
using std::forward;
using std::move;
}
#else

namespace mystd {

// It seems to me that these are enough bare bones :).
using std::forward;
using std::move;


template <typename T>
struct hash;

// FNV hash
inline std::size_t fnv_hash(std::uint8_t *bytes, std::size_t size) {
    std::size_t r = 2166136261;
    for (std::size_t i = 0; i < size; i++) {
        r ^= bytes[i];
        r *= 16777619;
    }
    return r;
}

template<>
struct hash<int> {
    std::size_t operator()(int i) const noexcept {
        return fnv_hash(reinterpret_cast<std::uint8_t *>(&i), sizeof(i));
    }
};

template<>
struct hash<unsigned long> {
    std::size_t operator()(unsigned long i) const noexcept {
        return fnv_hash(reinterpret_cast<std::uint8_t *>(&i), sizeof(i));
    }
};

}

#endif // FMI_OOP_USE_MYSTD_UTILITY

#endif // FMI_OOP_MYSTD_UTILITY_H
