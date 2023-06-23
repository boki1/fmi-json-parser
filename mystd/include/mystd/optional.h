#ifndef FMI_OOP_MYSTD_OPTIONAL_H
#define FMI_OOP_MYSTD_OPTIONAL_H

// There is defined the "toggle" macro.
#include <mystd/enable.h>
#include <mystd/utility.h>
#include <mystd/memory.h>

#include <optional> // for std::bad_optional_access

#ifndef FMI_OOP_USE_MYSTD_OPTIONAL

namespace mystd {
using std::optional;
}

#else

namespace mystd {


///
/// Small implementation of a type similar to `std::optinonal`. In this one
/// `mystd::unique_ptr<T>` is used as a container for the actual type T.
///
/// "The class template std::optional manages an optional contained value, i.e. a value that
/// may or may not be present."
/// -- cppreference
///

template <typename T>
class optional final {

public:

    ///
    /// Special member functions
    ///

    optional() noexcept = default;

    optional(const optional &other)
        : m_has_value{other.m_has_value}
    {
        if (other.m_has_value)
            m_ptr.reset(mystd::move(other.m_ptr.get()));
    }

    optional& operator=(const optional &other) {
        if (m_has_value)
            reset();
        m_has_value = other.m_has_value;
        if (m_has_value)
            m_ptr.reset(mystd::move(other.m_ptr.get()));
    }

    optional(optional &&other) noexcept
        : m_has_value{other.m_has_value}
    {
        if (m_has_value)
            m_ptr.reset(mystd::move(other.m_ptr.get()));
        other.m_has_value = false;
    }

    optional& operator=(optional &&other) noexcept {
        if (m_has_value)
            reset();
        m_has_value = other.m_has_value;
        if (m_has_value)
            m_ptr.reset(mystd::move(other.m_ptr.get()));
        other.m_has_value = false;
    }

    ~optional() noexcept = default;

    ///

    optional(std::nullopt_t) noexcept {}

    template <typename U = T>
    optional &operator=(U &&value) {
        if (m_has_value)
            reset();
        **this = mystd::forward<U&&>(value);
        m_has_value = true;
    }

    template< class... Args >
    explicit optional(std::in_place_t, Args&&... args)
        : m_has_value{true}
        , m_ptr{mystd::make_unique<T>(mystd::forward<Args>(args)...)}
    {}

    ///
    /// This is the trickiest part of this implementation, just like any other
    /// `optional`-type. This is well documented in cppreference "constructors" page
    /// regarding ctor (8):
    ///
    /// 8) Constructs an optional object that contains a value, initialized as if direct-initializing
    /// (but not direct-list-initializing) an object of type T with the expression std::forward<U>(value).
    ///     - If the selected constructor of T is a constexpr constructor, this constructor is a constexpr constructor.
    ///     - This constructor does not participate in overload resolution unless the following conditions are met:
    ///         + std::is_constructible_v<T, U&&> is true.
    ///         + std::decay_t<U> (until C++20) std::remove_cvref_t<U> (since C++20) is neither std::in_place_t nor std::optional<T>.
    ///         + If T is (possibly cv-qualified) bool, std::decay_t<U> (until C++20)std::remove_cvref_t<U> (since C++20) is
    ///           not a specialization of std::optional.
    ///     - This constructor is explicit if and only if std::is_convertible_v<U&&, T> is false.
    ///
    /// TL;DR In order to support syntax like `optional<int> a = 3;` we need some "implicit" conversion ctors,
    /// however we must take care to not be a better fit than the copy and move ctors.

    template<class U, std::enable_if_t<
        std::is_constructible_v<T,U&&> && std::is_convertible_v<U&&,T>, int> = 0>
    optional(U &&value)
        : m_has_value{true}
        , m_ptr{mystd::make_unique<T>(mystd::forward<U>(value))}
    {}

    template<class U, std::enable_if_t<
        std::is_constructible_v<T,U&&> && !std::is_convertible_v<U&&,T>, int> = 0>
    explicit optional(U &&value)
        : m_has_value{true}
        , m_ptr{mystd::make_unique<T>(mystd::forward<U>(value))}
    {}

    ///

    [[nodiscard]] bool operator==(const optional &other) const {
        return m_has_value == other.has_value
            && m_has_value
            && **this == *other;
    }

    [[nodiscard]] bool operator!=(const optional &other) const {
        return !(*this == other);
    }

public:

    ///
    /// Observers
    ///

    const T* operator->() const noexcept {
        return m_ptr.get();
    }

    T* operator->() noexcept {
        return m_ptr.get();
    }

    [[nodiscard]] const T& operator*() const& {
        return *m_ptr;
    }

    [[nodiscard]] T& operator*() & noexcept {
        return *m_ptr;
    }

    // Does this even make any sense?!
    // const T&& cannot be moved because of the const, right?
    [[nodiscard]] const T&& operator*() const&& noexcept {
        return std::move(*m_ptr);
    }

    [[nodiscard]] T&& operator*() && noexcept {
        return std::move(*m_ptr);
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    [[nodiscard]] bool has_value() const noexcept {
        return m_has_value;
    }

    [[nodiscard]] T& value() & {
        if (!m_has_value)
            throw std::bad_optional_access();
        return *m_ptr;
    }

    [[nodiscard]] const T& value() const & {
        if (!m_has_value)
            throw std::bad_optional_access();
        return *m_ptr;
    }

    [[nodiscard]] T&& value() && {
        if (!m_has_value)
            throw std::bad_optional_access();
        return std::move(*m_ptr);
    }

    [[nodiscard]] const T&& value() const && {
        if (!m_has_value)
            throw std::bad_optional_access();
        return std::move(*m_ptr);
    }

    template <class U>
    [[nodiscard]] T value_or(U &&default_value) const & {
        return bool(*this) ? **this
                           : static_cast<T>(mystd::forward<U>(default_value));
    }

    template <class U>
    [[nodiscard]] T value_or(U &&default_value) && {
        return bool(*this) ? std::move(**this)
                    : static_cast<T>(mystd::forward<U>(default_value));
    }

    void reset() noexcept {
        if (m_has_value)
            m_ptr.reset();
    }

    template <class... Args>
    T &emplace(Args &&...args) {
        if (m_has_value)
            reset();
        m_ptr = mystd::make_unique<T>(std::forward<Args>(args)...);
        return value();
    }

  private:
    bool m_has_value{false};
    mystd::unique_ptr<T> m_ptr;
};

} // namespace mystd

#endif  // FMI_OOP_USE_MYSTD_OPTIONAL

#endif  //FMI_OOP_MYSTD_OPTIONAL_H
