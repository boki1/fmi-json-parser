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

#include <vector>
#include <stdexcept>

#include <mystd/utility.h>
#include <mystd/algorithm.h>

namespace mystd {


///
/// This is the worst implementation I have done - fake unordered_map.
/// However, as you might expect this isn't exactly the most important part
/// of the implementation so I consider it to be functional enough.
///

template <typename Key, typename Val>
class fake_unordered_map final {

    using keys_type = std::vector<Key>;
    using vals_type = std::vector<Val>;

public:

    /// Special members functions
    /// No fancy ctors are available, so this type obeys the rule of 0.


    [[nodiscard]] bool operator==(const fake_unordered_map &rhs) const {
        // What would be a fake_unordered_map without such "efficient" comparator?
        return m_keys == rhs.m_keys && m_vals == rhs.m_vals;
    }

    [[nodiscard]] bool operator!=(const fake_unordered_map &rhs) const {
        return !(*this == rhs);
    }

    ///
    /// Iterators
    ///

    struct const_pair_type {
        const Key &first;
        const Val &second;
    };

    struct mut_pair_type {
        Key &first;
        Val &second;
    };

    using reference = mut_pair_type;
    using const_reference = const const_pair_type;

    class iterator {
        using keys_it_type = typename keys_type::iterator;
        using vals_it_type = typename vals_type::iterator;

    public:
        iterator(keys_it_type keys_it, vals_it_type vals_it)
            : m_keys_it{keys_it}
            , m_vals_it{vals_it} {}

        [[nodiscard]] bool operator==(const iterator &rhs) const {
            return m_keys_it == rhs.m_keys_it && m_vals_it == rhs.m_vals_it;
        }

        [[nodiscard]] bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }

        [[nodiscard]] const_reference operator*() const& {
            return const_pair_type {
                .first = *m_keys_it,
                .second = *m_vals_it
            };
        }

        [[nodiscard]] reference operator*() & {
            return mut_pair_type {
                .first = *m_keys_it,
                .second = *m_vals_it
            };
        }

        iterator &operator++() {
            ++m_keys_it;
            ++m_vals_it;
            return *this;
        }

        iterator operator++(int) {
            return iterator{m_keys_it++, m_vals_it++};
        }

    private:
        keys_it_type m_keys_it;
        vals_it_type m_vals_it;
    };

    class const_iterator {
        using keys_it_type = typename keys_type::const_iterator;
        using vals_it_type = typename vals_type::const_iterator;

    public:
        const_iterator(keys_it_type keys_it, vals_it_type vals_it)
            : m_keys_it{keys_it}
            , m_vals_it{vals_it} {}

        [[nodiscard]] bool operator==(const const_iterator &rhs) const {
            return m_keys_it == rhs.m_keys_it && m_vals_it == rhs.m_vals_it;
        }

        [[nodiscard]] bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }

        [[nodiscard]] const_reference operator*() const& {
            return const_pair_type {
                .first = *m_keys_it,
                .second = *m_vals_it
            };
        }

        const_iterator &operator++() {
            ++m_keys_it;
            ++m_vals_it;
            return *this;
        }

        const_iterator operator++(int) {
            return const_iterator{m_keys_it++, m_vals_it++};
        }

    private:
        keys_it_type m_keys_it;
        vals_it_type m_vals_it;
    };

    [[nodiscard]] iterator begin() {
        return iterator { m_keys.begin(), m_vals.begin() };
    }

    [[nodiscard]] iterator end() {
        return iterator { m_keys.end(), m_vals.end() };
    }

    [[nodiscard]] const_iterator begin() const {
        return const_iterator { m_keys.cbegin(), m_vals.cbegin() };
    }

    [[nodiscard]] const_iterator end() const {
        return const_iterator { m_keys.cend(), m_vals.cend() };
    }


    [[nodiscard]] const_iterator cbegin() const {
        return const_iterator { m_keys.cbegin(), m_vals.cbegin() };
    }

    [[nodiscard]] const_iterator cend() const {
        return const_iterator { m_keys.cend(), m_vals.cend() };
    }

    ///
    /// Capacity
    ///

    [[nodiscard]] bool empty() const noexcept {
        return m_keys.empty();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        assert(m_keys.size() == m_vals.size());
        return m_keys.size();
    }

    [[nodiscard]] std::size_t max_size() const noexcept {
        assert(m_keys.capacity() == m_vals.capacity());
        return m_keys.capacity();
    }

    ///
    /// Lookup
    ///

    [[nodiscard]] Val &at(const Key &key) {
        const auto key_it = mystd::find(std::cbegin(m_keys), std::cend(m_keys), key);
        if (key_it == std::cend(m_keys))
            throw std::out_of_range("Indexing fake_unordered_map with non-existing key.");
        const auto val_it = std::begin(m_vals) + (key_it - std::cbegin(m_keys));
        return *val_it;
    }

    [[nodiscard]] const Val &at(const Key &key) const {
        const auto key_it = mystd::find(std::cbegin(m_keys), std::cend(m_keys), key);
        if (key_it == std::cend(m_keys))
            throw std::out_of_range("Indexing fake_unordered_map with non-existing key.");
        const auto val_it = std::cbegin(m_vals) + (key_it - std::cbegin(m_keys));
        return *val_it;
    }

    [[nodiscard]] Val& operator[](const Key& key) {
        const auto key_it = mystd::find(std::cbegin(m_keys), std::cend(m_keys), key);
        if (key_it == std::cend(m_keys)) {
            m_keys.emplace_back(key);
            m_vals.emplace_back(); // default
            return m_vals.back();
        }

        const auto val_it = std::cbegin(m_vals) + (key_it - std::cbegin(m_keys));
        return *val_it;
    }

    [[nodiscard]] Val& operator[](Key&& key) {
        const auto key_it = mystd::find(std::cbegin(m_keys), std::cend(m_keys), key);
        if (key_it == std::cend(m_keys)) {
            m_keys.emplace_back(mystd::move(key));
            m_vals.emplace_back(); // default
            return m_vals.back();
        }

        const auto val_it = std::begin(m_vals) + (key_it - std::cbegin(m_keys));
        return *val_it;
    }

    [[nodiscard]] std::size_t count(const Key& key) const {
        return mystd::count(std::cbegin(m_keys), std::cend(m_keys), key);
    }

    [[nodiscard]] iterator find(const Key& key) {
        const auto key_it = mystd::find(std::begin(m_keys), std::end(m_keys), key);
        const auto val_it = std::begin(m_vals) + (key_it - std::begin(m_keys));
        return iterator { key_it, val_it };
    }

    [[nodiscard]] const_iterator find(const Key& key) const {
        const auto key_it = mystd::find(std::cbegin(m_keys), std::end(m_keys), key);
        const auto val_it = std::cbegin(m_vals) + (key_it - std::begin(m_keys));
        return iterator { key_it, val_it };
    }

    bool contains(const Key& key) const {
        return mystd::find(std::cbegin(m_keys), std::cend(m_keys), key) != std::cend(m_keys);
    }

    ///
    /// Modifiers
    ///

    void clear() noexcept {
        m_keys.clear();
        m_vals.clear();
    }

    /// Here is a difference between the std::unordered_map and this one.
    /// std::unordered_map::emplace returns a pair of iterator and a bool,
    /// but I am not using this return value in the library and in order
    /// to define it this way I would have to implement mystd::pair so I
    /// will leave it be void. Also the input parameters are somewhat
    /// different.

    template <typename KeyArg, typename ValArg>
    void emplace(KeyArg &&key_arg, ValArg &&val_arg) {
        m_keys.emplace_back(mystd::forward<KeyArg>(key_arg));
        m_vals.emplace_back(mystd::forward<ValArg>(val_arg));
    }

    std::size_t erase(const Key& key) {
        const auto key_it = mystd::find(std::cbegin(m_keys), std::cend(m_keys), key);
        if (key_it == std::cend(m_keys))
            return 0;
        const auto val_it = std::cbegin(m_vals) + (key_it - std::cbegin(m_keys));
        m_keys.erase(key_it);
        m_vals.erase(val_it);
        return 1;
    }

private:
    keys_type m_keys;
    vals_type m_vals;
};

template <typename K, typename V, typename H = void>
using unordered_map = fake_unordered_map<K, V>;


}

#endif  // FMI_OOP_USE_MYSTD_UNORDERED_MAP

#endif  //FMI_OOP_MYSTD_UNORDERED_MAP_H
