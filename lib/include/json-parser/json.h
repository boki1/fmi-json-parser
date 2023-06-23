#ifndef FMI_JSON_PARSER_JSON_INCLUDED
#define FMI_JSON_PARSER_JSON_INCLUDED

#include <string>
#include <vector>
#include <iostream>
#include <concepts>
#include <stdexcept>

#include <mystd/unordered_map.h>
#include <mystd/memory.h>
#include <mystd/type_traits.h>
#include <mystd/utility.h>

#include <json-parser/tokenizer.h>

namespace json_parser {

///
/// The `stringable` concept.
/// It is satisfied by "string-like" types such as `std::string`,
/// `char *` and `const char *`. It gets used for the operator[]
/// of the `json::object` type.

template<class T>
concept stringable = std::is_convertible_v<T, std::string_view>;

static_assert(stringable<std::string>);
static_assert(stringable<char *>);
static_assert(stringable<const char *>);

///
/// The `json_exception` class.
/// It is reponsible for reporting errors that happen when misusing
/// `json` instances. Note that it is in no way a replacement of
/// `parsed_exception` which is responsible for the parsing process's
/// error reporting. The `json_exception` is used only _after_ the
/// input stream has been parsed and there is an actual in-memory
/// representation of a _valid_ JSON object.
///
class json_exception : public std::exception {
public:
    explicit json_exception(std::string msg)
        : m_msg { std::move(msg) }
    {
    }

    [[nodiscard]] const char* what() const noexcept { return m_msg.c_str(); }

private:
    std::string m_msg;
};

///
/// The `json` class.
/// This is the main type used to represent a valid JSON object.
/// An instance of it gets created using the parsing process by
/// the `parser`.
///
class json {

    static constexpr inline size_t serialization_tab_size = 2;

public:
    ///
    /// JSON Types.
    ///

    ///
    /// This is the base type used to represent a JSON value.
    /// According to "json.org" each token that is found in a valid JSON
    /// source is part of a value. There are multiple kinds of them:
    /// the "trivial" ones - boolean, null, number and string, as well as
    /// the "compound" ones - array and object. As `value` is useful in the
    /// context of the type hierarchy it established, it usually gets used
    /// as a "polymorphic" value in the form the alias `pmrvalue`.
    ///
    class value;
    using pmrvalue = mystd::unique_ptr<value>;

    class value {
    public:
        virtual ~value() noexcept {}

        // The idea for this implementation of comparison in hierarchy is taken from here:
        // https://stackoverflow.com/questions/13045399/how-to-implement-operator-for-polymorphic-classes-in-c#13045492.
        [[nodiscard]] friend bool operator==(const value &lhs, const value &rhs) noexcept {
            return typeid(lhs) == typeid(rhs) && lhs.equals(rhs);
        }

        [[nodiscard]] friend bool operator!=(const value &lhs, const value &rhs) noexcept {
            return !(lhs == rhs);
        }

        virtual bool equals(const value &) const noexcept { return true; }

        virtual void serialize(std::ostream &os, std::size_t depth, bool in_object = false) const = 0;
        virtual json::pmrvalue clone() const = 0;
        virtual bool trivial() const = 0;
        virtual bool compound() const = 0;
    };

    ///
    /// Trivial JSON types.
    ///
    /// These are not really interesting in terms of implementation. The only
    /// notable difference between their corresponding tokens (in the `token`
    /// hierarchy) is the separation of a "keyword" into `boolean` and `null`.
    /// This was primarily done for ease of use in the end-used API as the
    /// author of this library found out how ugly it is otherwise :D.
    ///

    class trivial_value : public value {
    public:
        bool trivial() const override { return true; }
        bool compound() const override { return false; }
    };

    class boolean : public trivial_value {
    public:
        ///
        /// Special member functions.
        ///

        explicit boolean(token_keyword data)
            : m_data{std::move(data)} {}

        /// The `json::boolean` is implicitly convertible to the native `bool` C++ type.
        boolean(bool data)
            : m_data{data ? token_keyword::kind::True
                          : token_keyword::kind::False} {}

    private:
        /// Polymorphic comparator
        [[nodiscard]] bool equals(const value &rhs) const noexcept override {
            const boolean* rhs_as_boolean = dynamic_cast<const boolean*>(&rhs);
            if(rhs_as_boolean == nullptr)
                return false;
            return value::equals(rhs) && m_data == rhs_as_boolean->m_data;
        }

    public:

        operator bool() const { return m_data.value() == token_keyword::kind::True; }

        ///
        /// Common behaviour for the `value` types:
        ///

        void serialize(std::ostream &os, std::size_t depth, bool in_object = false) const override;

        json::pmrvalue clone() const override {
            return mystd::make_unique<boolean>(*this);
        }

    private:
        token_keyword m_data;
    };

    class null : public trivial_value {
    private:
        /// Polymorphic comparator
        /// All `null`s are the identical.
        [[nodiscard]] bool equals(const value &rhs) const noexcept override {
            const null* rhs_as_null = dynamic_cast<const null*>(&rhs);
            if(rhs_as_null == nullptr)
                return false;
            return value::equals(rhs);
        }

    public:
        operator bool() const { return false; }

        ///
        /// Common behaviour for the `value` types:
        ///

        void serialize(std::ostream &os, std::size_t depth, bool in_object = false) const override;

        json::pmrvalue clone() const override { return mystd::make_unique<null>(*this); }


    private:
        token_keyword m_data{token_keyword::kind::Null};
    };

    class number : public trivial_value {
    public:
        ///
        /// Special member functions.
        ///

        explicit number(token_number data)
            : m_data{std::move(data)} {}

        /// The `json::number` is implicitly convertible to the native `double` C++ type.
        number(double data)
            : m_data{std::move(data)} {}

        operator double() const { return m_data.value(); }

    private:
        /// Polymorphic comparator
        [[nodiscard]] bool equals(const value &rhs) const noexcept override {
            const number* rhs_as_number = dynamic_cast<const number*>(&rhs);
            if(rhs_as_number == nullptr)
                return false;
            return value::equals(rhs) && m_data == rhs_as_number->m_data;
        }

    public:

        ///
        /// Common behaviour for the `value` types:
        ///

        void serialize(std::ostream &os, std::size_t depth, bool in_object = false) const override;

        json::pmrvalue clone() const override { return mystd::make_unique<number>(*this); }

    private:
        token_number m_data;
    };

    class string : public trivial_value {
    public:
        ///
        /// Special member functions.
        ///

        explicit string(token_string data)
            : m_data{std::move(data)} {}

        /// The `json::string` is implicitly convertible to the `std::string` C++ type.
        string(const char *data)
            : m_data{std::string{data}} {}

        string(const std::string& data)
            : m_data{std::move(data)} {}

        explicit operator std::string() const { return m_data.value(); }

    private:
        /// Polymorphic comparator
        [[nodiscard]] bool equals(const value &rhs) const noexcept override {
            const string* rhs_as_string = dynamic_cast<const string*>(&rhs);
            if(rhs_as_string == nullptr)
                return false;
            return value::equals(rhs) && m_data == rhs_as_string->m_data;
        }

    public:

        [[nodiscard]] friend bool operator==(const json::string &lhs, const std::string &rhs) noexcept {
            return lhs.m_data.value() == rhs;
        }

        [[nodiscard]] friend bool operator!=(const json::string &lhs, const std::string &rhs) noexcept {
            return !(lhs == rhs);
        }

        [[nodiscard]] friend bool operator==(const std::string &rhs, const json::string &lhs) noexcept {
            return lhs == rhs;
        }

        [[nodiscard]] friend bool operator!=(const std::string &rhs, const json::string &lhs) noexcept {
            return lhs != rhs;
        }

        ///
        /// Common behaviour for the `value` types:
        ///

        void serialize(std::ostream &os, std::size_t depth, bool in_object = false) const override;

        json::pmrvalue clone() const override { return mystd::make_unique<string>(*this); }

        const value *find(const trivial_value &target) const override;
        value *find(const trivial_value &target) override;

    public:
        /// json::string has to be hashable, because it is the key type of
        /// `json::object`'s inner map container.
        struct hasher {
            size_t operator()(const json::string &s) const {
                return std::hash<std::string>{}(s.m_data.value());
            }
        };

    private:
        token_string m_data;
    };

    ///
    /// Compound JSON types.
    ///
    /// These are values which contain other values. Therefore as they have
    /// similar properties I implemented them as derived classes of a
    /// template `container_value`. This was not really mandatory but in my
    /// opinion shortened the code, althouth I might have traded off for
    /// readability because of the (someplace) weird generics used.

    template <typename DataType>
    class container_value : public value {
        friend json;

        using data_type = DataType;

    public:
        virtual ~container_value() noexcept = default;

    public:
        ///
        /// Common behaviour for the compound `value` types:
        ///

        bool trivial() const override { return false; }
        bool compound() const override { return true; }

        template <typename ...ItemType>
        void append(ItemType&& ...) { mystd::unreachable(); }

        [[nodiscard]] json::value &operator[](auto &&i) {
            auto &pmrval = m_data.at(mystd::forward<decltype(i)>(i));
            return *pmrval;
        }

        [[nodiscard]] const json::value &operator[](auto &&i) const {
            const auto &pmrval = m_data.at(mystd::forward<decltype(i)>(i));
            return *pmrval;
        }

        [[nodiscard]] typename data_type::const_iterator cbegin() const { return m_data.cbegin(); }
        [[nodiscard]] typename data_type::const_iterator cend() const { return m_data.cend(); }

        [[nodiscard]] typename data_type::iterator begin() { return m_data.begin(); }
        [[nodiscard]] typename data_type::iterator end() { return m_data.end(); }

        [[nodiscard]] std::size_t size() const noexcept { return m_data.size(); }
        [[nodiscard]] std::size_t empty() const noexcept { return m_data.empty(); }

    protected:
        data_type m_data;
    };

    class object : public container_value<
                       mystd::unordered_map<json::string, pmrvalue, json::string::hasher>>
    {
    public:
        ///
        /// Common behaviour for the `value` types:
        ///

        void serialize(std::ostream &os, std::size_t depth, bool in_object = false) const override {
            if (!in_object)
                os << std::string(depth, ' ');
            os << "{\n";
            size_t count = 0;
            for (const auto &[str, val] : m_data){
                str.serialize(os, depth + serialization_tab_size);
                os << " : ";
                val->serialize(os, depth + serialization_tab_size, true);
                if (++count < m_data.size())
                    os << ",\n";
            }
            os << "\n" << std::string(depth, ' ') << "}";
        }

        template <typename ...ItemType>
        void append(ItemType&& ...item_args) {
            m_data.emplace(mystd::forward<ItemType>(item_args)...);
        }

        json::pmrvalue clone() const override {
            auto cloned = mystd::make_unique<object>();
            for (const auto &[key, val] : m_data){
                auto cloned_key = key.clone();
                json::string *cloned_key_ptr = dynamic_cast<json::string *>(cloned_key.get());
                assert(cloned_key_ptr != nullptr);
                cloned->append(std::move(*cloned_key_ptr), val->clone());
            }
            return cloned;
        }

    private:
        /// Polymorphic comparator
        [[nodiscard]] bool equals(const value &rhs) const noexcept override {
            const object* rhs_as_object = dynamic_cast<const object*>(&rhs);
            if(rhs_as_object == nullptr)
                return false;
            return value::equals(rhs) && m_data == rhs_as_object->m_data;
        }
    };

    class array : public container_value<std::vector<pmrvalue>>
    {
    public:
        ///
        /// Common behaviour for the `value` types:
        ///

        void serialize(std::ostream &os, std::size_t depth, [[maybe_unused]] bool in_object = false) const override {
            if (!in_object)
                os << std::string(depth, ' ');
            os << "[\n";
            size_t count = 0;
            for (const auto &val : m_data) {
                val->serialize(os, depth + serialization_tab_size);
                if (++count < m_data.size())
                    os << ",\n";
            }
            os << "\n" << std::string(depth, ' ') << "]";
        }

        template <typename ...ItemType>
        void append(ItemType&& ...item_args) {
            m_data.emplace_back(mystd::forward<ItemType>(item_args)...);
        }

        json::pmrvalue clone() const override {
            auto cloned = mystd::make_unique<array>();
            for (const auto &val : m_data)
                cloned->append(val->clone());
            return cloned;
        }

    private:
        /// Polymorphic comparator
        [[nodiscard]] bool equals(const value &rhs) const noexcept override {
            const array* rhs_as_array = dynamic_cast<const array*>(&rhs);
            if(rhs_as_array == nullptr)
                return false;
            return value::equals(rhs) && m_data == rhs_as_array->m_data;
        }
    };

public:
    ///
    /// Accessors
    ///
    /// TODO: This is the most important future work I have in mind. However,
    /// I am not really sure how to implement it :/. I want to make the operator[]
    /// return _the most specific_ json::value derivation; e.g (in some pseudo-C++)
    ///
    /// json: { "Apple" : "123" }
    /// decltype(json["Apple"]) == json::string
    ///
    /// This would allow for nice chaining of indexing operations:
    /// json: { "Apple" : [ { "a" : 1, "b" : 2 }, { "A" : 1, "B" : 2 } ] }
    /// json["Apple"][1]["B"] == 2
    ///
    /// As of now, dynamic casts are required in the end-user's code.
    ///

    [[nodiscard]] const json::value &operator[](auto &&i) const {
        if constexpr (std::integral<std::decay_t<decltype(i)>>) {
            if (const auto *root_as_array = dynamic_cast<const json::array *>(m_root_node.get()); root_as_array)
                return (*root_as_array)[i];
        }

        if constexpr (stringable<decltype(i)>) {
            if (const auto *root_as_object = dynamic_cast<const json::object *>(m_root_node.get()); root_as_object)
                return (*root_as_object)[mystd::forward<decltype(i)>(i)];
        }

        throw json_exception("Cannot index a non-container JSON type");
    }

    [[nodiscard]] json::value &operator[](auto &&i) {
        if constexpr (std::integral<decltype(i)>) {
            if (auto *root_as_array = dynamic_cast<json::array *>(m_root_node.get()); root_as_array)
                return (*root_as_array)[i];
        }

        if constexpr (stringable<decltype(i)>) {
            if (auto *root_as_object = dynamic_cast<json::object *>(m_root_node.get()); root_as_object)
                return (*root_as_object)[mystd::forward<decltype(i)>(i)];
        }

        throw json_exception("Cannot index a non-container JSON type");
    }

public:
    ///
    /// Special member functions
    ///

    json() = default;

    json(const json &);
    json& operator=(const json &);

    json(json &&) noexcept = default;
    json& operator=(json &&) noexcept = default;

    explicit json(json::pmrvalue root_node)
        : m_root_node{std::move(root_node)} {}

    bool operator==(const json &) const noexcept;
    bool operator!=(const json &rhs) const noexcept { return !(*this == rhs); }

    ///
    /// Operations
    ///

    void dump(std::ostream &os) const;

    ///
    /// Properties
    ///

    [[nodiscard]] const json::value *root() const noexcept {
        return m_root_node.get();
    }

    [[nodiscard]] json::value *root() noexcept {
        return m_root_node.get();
    }

    [[nodiscard]] const json::value &root_unsafe() const {
        return *(m_root_node.get());
    }

    [[nodiscard]] json::value &root_unsafe() {
        return (*m_root_node.get());
    }

    [[nodiscard]] bool compound() const {
        if (m_root_node)
            return m_root_node->compound();
        return false;
    }

    [[nodiscard]] bool trivial() const {
        if (m_root_node)
            return m_root_node->trivial();
        return false;
    }

    [[nodiscard]] bool empty() const noexcept {
        return (bool) m_root_node;
    }

private:
    json::pmrvalue m_root_node;
};

///
/// Helpers
///

template <typename NodeType, typename ...T>
json::pmrvalue make_node(T&& ...args) {
    return mystd::make_unique<NodeType>(mystd::forward<T>(args)...);
}

} // namespace json_parser

#endif // FMI_JSON_PARSER_JSON_INCLUDED
