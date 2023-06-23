#ifndef FMI_JSON_PARSER_INPUT_READER_INCLUDED
#define FMI_JSON_PARSER_INPUT_READER_INCLUDED

#include <string>
#include <iostream>
#include <assert.h>
#include <exception>

namespace json_parser {

///
/// The `input_reader_exception` class.
/// The exception type that is used for exceptional behaviour in the
/// input reader strategy that is being used. In case such is thrown,
/// the `tokenizer` is responsible for catching it and rethrowing as
/// `token_exception` with the same message that gets initially passed.
///
class input_reader_exception : public std::exception {
public:
    explicit input_reader_exception(std::string msg)
        : m_msg { std::move(msg) } { }

    [[nodiscard]] const char* what() const noexcept { return m_msg.c_str(); }

private:
    std::string m_msg;
};

///
/// The `input_reader` class.
/// Abstract base type for different input reader strategies.
/// One of them is passed as a template argument to the `tokenizer`
/// instance that is used. Its role is to serve as an _algorithm strategy_
/// for acquiring input and reporting problems.
/// The `input_reader` base is implemented using CRTP in order to be able
/// to have an API for creating `begin()`/`end()` instances of the _concrete_
/// type.
template <typename Concrete>
class input_reader {
public:
    virtual ~input_reader() noexcept = default;

    virtual Concrete begin() const = 0;
    virtual Concrete end() const = 0;

    virtual bool eof() const = 0;
    virtual bool ready() const = 0;

    // TODO: Support non-`char` types also.
    virtual char peek() const = 0;
    virtual char get() = 0;

    using pos_type = std::size_t;

    virtual void seek(pos_type) = 0;
    virtual pos_type tell() const = 0;

    // I cannot put such requirements here, but the derived classes actually have these properties also:
    //
    // virtual const Concrete::impl &impl() const = 0;
    // virtual Concrete::impl &impl() = 0;
    //
    // using location_detail = std::size_t;
};

///
/// The `ifs_input_reader` class.
/// This type implements using a file as an input stream.
/// On construction guarantees that the input is "ready".
///
class ifs_input_reader final : public input_reader<ifs_input_reader> {
public:
    explicit ifs_input_reader(const std::string &ifs_filename)
        : m_ifs_filename{ifs_filename}
        , m_ifs{ifs_filename} {
        if (!ready())
            throw input_reader_exception{"Cannot open input '" + m_ifs_filename + "'of type `ifs_input_reader`."};
    }

    ifs_input_reader(const ifs_input_reader &rhs)
        : m_ifs_filename{rhs.m_ifs_filename}
        , m_ifs{rhs.m_ifs_filename} {
        if (!ready())
            throw input_reader_exception{"Cannot open input '" + m_ifs_filename + "'of type `ifs_input_reader`."};
    }

    ifs_input_reader& operator=(const ifs_input_reader &rhs) {
        m_ifs_filename = rhs.m_ifs_filename;
        m_ifs = std::ifstream { rhs.m_ifs_filename };
        return *this;
    }

    ifs_input_reader(ifs_input_reader &&) noexcept = default;

    ifs_input_reader& operator=(ifs_input_reader &&) noexcept = default;

    [[nodiscard]] ifs_input_reader begin() const override {
        ifs_input_reader copy { *this };
        copy.m_ifs.seekg(0, std::ios_base::beg);
        return copy;
    }

    [[nodiscard]] ifs_input_reader end() const override {
        ifs_input_reader copy { *this };
        copy.m_ifs.seekg(0, std::ios_base::end);
        return copy;
    }

    [[nodiscard]] bool ready() const override { return m_ifs.is_open() && m_ifs.good(); }

    [[nodiscard]] bool eof() const override { return m_ifs.eof(); }

    [[nodiscard]] char peek() const override { return m_ifs.peek(); }

    [[nodiscard]] char get() override { return m_ifs.get(); }

    void seek(pos_type pos) override { m_ifs.seekg(pos, std::ios_base::beg); }

    [[nodiscard]] pos_type tell() const override { return m_ifs.tellg(); }

    using impl_type = std::ifstream;
    [[nodiscard]] const impl_type &ifs() const { return m_ifs; }
    [[nodiscard]] impl_type &ifs() { return m_ifs; }

    [[nodiscard]] const std::string &ifs_filename() const { return m_ifs_filename; }

    [[nodiscard]] static const std::string &kind() noexcept {
        static std::string kind = "ifs_input_reader";
        return kind;
    }

private:
    std::string m_ifs_filename;

    // This is the actual input stream.
    // It is mutable because of the `tell()` and `peek()` methods.
    mutable std::ifstream m_ifs;
};

///
/// The `str_input_reader` class.
/// This type implements using a raw byte buffer as an input stream. Here the byte buffer
/// is implemented as a `std::string` as "regular" `char`-s are all we support for now in
/// terms of character sets.
///
class str_input_reader final : public input_reader<str_input_reader> {
public:
    explicit str_input_reader(const std::string &str) : m_str{str} {
        // On construction the input is guaranteed to be "ready".
        // Safety: str_input_reader is always ready!
        assert(ready());
    }

    [[nodiscard]] str_input_reader begin() const override {
        str_input_reader copy { m_str };
        copy.seek(0);
        return copy;
    }

    [[nodiscard]] str_input_reader end() const override {
        str_input_reader copy { m_str };
        copy.seek(m_str.size());
        return copy;
    }

    [[nodiscard]] bool ready() const override {
        // Strings are always ready :D.
        return true;
    }

    [[nodiscard]] bool eof() const override { return !(m_pos < m_str.size()); }

    [[nodiscard]] char peek() const override {
        if (m_pos < m_str.size())
            return m_str[m_pos];
        throw input_reader_exception("Trying to peek at str_input_reader out of bounds.");
    }

    [[nodiscard]] char get() override {
        if (m_pos < m_str.size())
            return m_str[m_pos++];
        throw input_reader_exception("Trying to get from str_input_reader out of bounds.");
    }

    void seek(pos_type pos) override { m_pos = pos; }

    [[nodiscard]] pos_type tell() const override { return m_pos; }

    [[nodiscard]] static const std::string &kind() noexcept {
        static std::string kind = "str_input_reader";
        return kind;
    }

private:
    std::string m_str;
    pos_type m_pos{0};
};

///
/// The `is_input_reader` and `is_input_reader_v` traits.
///
/// They are not really required by the current implementation but I think
/// they are somewhat usefulfor detecting incorrect "generated" code. For
/// example I was debugging an instantiation of `input_reader<std::string>`
/// and was wondering what is going on, so I decided to limit the template
/// arguments with this cool new think I learned some days ago :D.

template <typename T>
struct is_input_reader {
    static constexpr bool value = std::is_base_of_v<input_reader<T>, T>;
};

template <typename T>
inline constexpr bool is_input_reader_v = is_input_reader<T>::value;

static_assert(is_input_reader_v<str_input_reader>);
static_assert(is_input_reader_v<ifs_input_reader>);

}

#endif // FMI_JSON_PARSER_INPUT_READER_INCLUDED
