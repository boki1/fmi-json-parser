#ifndef FMI_OOP_MYSTD_MEMORY_H
#define FMI_OOP_MYSTD_MEMORY_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>
#include <mystd/utility.h>

#include <cmath> // for std::size_t

#ifdef FMI_OOP_USE_MYSTD_MEMORY

#include <memory>

namespace mystd {

using std::unique_ptr;
using std::make_unique;

}

#else

namespace mystd {

///
/// Implementation of type similar to the single-object version of `std::unique_ptr`.
///
/// "std::unique_ptr is a smart pointer that owns and manages another object through
/// a pointer and disposes of that object when the unique_ptr goes out of scope."
/// -- cppreference
template <typename T>
class unique_ptr final {

public:

    using pointer = T *;
    ///
    /// Special member functions
    ///

    unique_ptr() = default;

    unique_ptr(std::nullptr_t) noexcept
        : m_ptr{nullptr} {}

    explicit unique_ptr(pointer ptr) noexcept
        : m_ptr{ptr} {}

    /// unique_ptr is not copyable
    unique_ptr(const unique_ptr &) = delete;
    unique_ptr &operator=(const unique_ptr &) = delete;

    template <typename U>
    unique_ptr(unique_ptr<U> &&rhs)
        : m_ptr{rhs.get()}
    {
        rhs.reset();
    }

    template <typename U>
    unique_ptr& operator=(unique_ptr<U> &&rhs) noexcept {
        if (*this == rhs)
            return *this;
        m_ptr = rhs.get();
        rhs.reset();
        return *this;
    }

    ~unique_ptr() noexcept {
        delete m_ptr;
    }

    [[nodiscard]] auto operator<=>(const unique_ptr &) const noexcept = default;

    ///
    /// Modifiers
    ///

    void swap(unique_ptr& other) noexcept {
        T *tmp = m_ptr;
        m_ptr = other.m_ptr;
        other.m_ptr = tmp;
    }

    void reset(pointer new_ptr = nullptr) noexcept {
        delete m_ptr;
        m_ptr = new_ptr;
    }

    pointer release() noexcept {
        T *tmp = m_ptr;
        m_ptr = nullptr;
        return tmp;
    }

    ///
    /// Observers
    ///

    pointer get() const noexcept {
        return m_ptr;
    }

    pointer get() noexcept {
        return m_ptr;
    }

    pointer operator->() const noexcept {
        return m_ptr;
    }

    pointer operator->() noexcept {
        return m_ptr;
    }

    const T& operator*() const {
        return *m_ptr;
    }

    T& operator*() {
        return *m_ptr;
    }

    explicit operator bool() const noexcept { return m_ptr != nullptr; }

private:
    T *m_ptr;
};

/// Make unique_ptr<T> hashable using mystd::hash<T>.

template <typename T>
struct hash<unique_ptr<T>> {
    size_t operator()(const mystd::unique_ptr<T> &up) const noexcept {
        return static_cast<unsigned long>(up.get());
    }
};

/// Helper
template <class T, class... Args>
unique_ptr<T> make_unique(Args &&...args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace mystd

#endif  // FMI_OOP_USE_MYSTD_MEMORY

#endif  //FMI_OOP_MYSTD_MEMORY_H
