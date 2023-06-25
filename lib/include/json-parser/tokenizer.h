#ifndef FMI_JSON_PARSER_TOKENIZER_INCLUDED
#define FMI_JSON_PARSER_TOKENIZER_INCLUDED

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include <mystd/memory.h>
#include <mystd/optional.h>
#include <mystd/utility.h>

#include <json-parser/input_reader.h>

namespace json_parser {

///
/// The `location` class.
/// This type describes a location in two ways: a human-friendly line-column way,
/// and a `input-reader`-friendly way using a secret `detail_pos` type that
/// is the internal way of storing location for the specific input. This type is
/// very useful during the error reporting process.
///

class location {
public:
    using detail_type = std::size_t;

    explicit location(detail_type detail_pos)
        : m_detail_pos { detail_pos } { }

    // This comparison operations are the ones used to compare the type `token_citerator`.
    // Of course, being a "detail", these implementations are somewhat as a "wrappers",
    // altough currently the "detail" type is plain std::size_t, but shh - that is a secret! :D

    [[nodiscard]] bool operator==(const location &rhs) const noexcept {
        return m_detail_pos == rhs.m_detail_pos;
    }

    [[nodiscard]] std::strong_ordering operator<=>(const location &rhs) const noexcept {
        return m_detail_pos <=> rhs.m_detail_pos;
    }

	[[nodiscard]] std::string to_string() const
	{
		std::string s;
        s += "Line: ";
		s += std::to_string(m_line_num);
        s += ", Column: ";
		s += std::to_string(m_column_num);
        s += " (Detail-specific: ";
        s += std::to_string(m_detail_pos);
		s += ")";
		return s;
	}

    [[nodiscard]] std::size_t line_num() const& noexcept { return m_line_num; }
    [[nodiscard]] std::size_t column_num() const& noexcept { return m_column_num; }
    [[nodiscard]] detail_type detail_pos() const& noexcept { return m_detail_pos; }

    [[nodiscard]] std::size_t& line_num() & noexcept { return m_line_num; }
    [[nodiscard]] std::size_t& column_num() & noexcept { return m_column_num; }
    [[nodiscard]] detail_type& detail_pos() & noexcept { return m_detail_pos; }

private:
    // These two store location information in human-friendly way
    // using line and column number.
    std::size_t m_line_num { 0 };
	std::size_t m_column_num { 0 };

    // The `m_detail_pos` property is used as a `m_input_reader`-friedly way
    // to store a location. It is specific to the concrete input reader
    // implementation.
    detail_type m_detail_pos;
};

///
/// The `token_exception` class.
/// This type is used to signify exception behvaiour during the tokenization process.
/// It is considered somewhat low-level so it is hidden by the `parser_exception` type
/// later during the whole parsing process in the same way that the `input_reader_exception`
/// is hidden by this one. Often the exception is associated with a location in the
/// concrete input so it is stored.
///

class token_exception : public std::exception {

public:
    explicit token_exception(std::string msg,
                              mystd::optional<location> location = {})
        : m_location { mystd::move(location) }
        , m_msg { mystd::move(msg) }
    {
    }

    virtual ~token_exception() = default;

	[[nodiscard]] const char* what() const noexcept { return m_msg.c_str(); }

private:
	mystd::optional<location> m_location;
	std::string m_msg;
};

///
/// The token type hierarchy.
/// There is base class `token` - it holds no data.
/// The concrete tokens are related to the different lexemes that
/// could be encountered during tokenization of correct JSON input.
///

struct token {
	virtual ~token() noexcept = default;

    /// Same as in the `json::value` hierarchy.
    [[nodiscard]] friend bool operator==(const token &lhs, const token &rhs) noexcept {
        return typeid(lhs) == typeid(rhs) && lhs.equals(rhs);
    }

    [[nodiscard]] friend bool operator!=(const token &lhs, const token &rhs) noexcept {
        return !(lhs == rhs);
    }

    virtual bool equals(const token &) const noexcept { return true; }

    virtual void serialize(std::ostream &os) const = 0;
    virtual mystd::unique_ptr<token> clone() const noexcept = 0;
};

class token_string final : public token {
public:
	explicit token_string(const std::string& value)
		: m_value { value }
	{
    }

private:
    bool equals(const token &rhs) const noexcept override {
        const token_string* rhs_as_string = dynamic_cast<const token_string*>(&rhs);
        if(rhs_as_string == nullptr)
            return false;
        return token::equals(rhs) && m_value == rhs_as_string->m_value;
    }

public:
	[[nodiscard]] const std::string& value() const noexcept { return m_value; }

    void serialize(std::ostream &os) const override;

    mystd::unique_ptr<token> clone() const noexcept override { return mystd::make_unique<token_string>(*this); }

private:
	std::string m_value;
};

class token_number final : public token {
public:
	explicit token_number(double value)
		: m_value { value }
	{
    }

	[[nodiscard]] double value() const noexcept { return m_value; }

    void serialize(std::ostream &os) const override;

    mystd::unique_ptr<token> clone() const noexcept override { return mystd::make_unique<token_number>(*this); }

private:
	double m_value;
};

class token_keyword final : public token {
public:
    enum class kind {
        True,
        False,
        Null
    };

    explicit token_keyword(kind value)
		: m_value { value }
	{
	}

private:
    bool equals(const token &rhs) const noexcept override {
        const token_keyword* rhs_as_keyword = dynamic_cast<const token_keyword*>(&rhs);
        if(rhs_as_keyword == nullptr)
            return false;
        return token::equals(rhs) && m_value == rhs_as_keyword->m_value;
    }

public:
    [[nodiscard]] kind value() const noexcept { return m_value; }

    void serialize(std::ostream &os) const override;

    mystd::unique_ptr<token> clone() const noexcept override { return mystd::make_unique<token_keyword>(*this); }

private:
    kind m_value;
};

class token_punct final : public token {
public:
	explicit token_punct(char value)
		: m_value { value }
	{
		// Safety: the token_punct ctor is called only with valid data!
		assert(token_punct::is_valid(value));
    }

private:
    bool equals(const token &rhs) const noexcept override {
        const token_punct* rhs_as_punct = dynamic_cast<const token_punct*>(&rhs);
        if(rhs_as_punct == nullptr)
            return false;
        return token::equals(rhs) && m_value == rhs_as_punct->m_value;
    }

public:
	[[nodiscard]] char value() const noexcept { return m_value; }

    void serialize(std::ostream& os) const override;

    mystd::unique_ptr<token> clone() const noexcept override { return mystd::make_unique<token_punct>(*this); }

    [[nodiscard]] static bool is_valid(char value);

private:
	char m_value;
};

///
/// Helper functions
///

template <typename Kind, typename Value = Kind>
mystd::unique_ptr<Kind> make_token(Value&& value)
{
	return mystd::make_unique<Kind>(mystd::forward<Value>(value));
}

template <typename T>
const T *token_as(const token *abstract_token) {
    return dynamic_cast<const T *const>(abstract_token);
}

template <typename T>
const T *token_as(const mystd::unique_ptr<token> &abstract_token) {
    return dynamic_cast<const T *const>(abstract_token.get());
}

///
/// The `token_citerator` class
/// This is the driver class for the tokenization process.
/// It holds the logic for token consuming. It is generic over
/// an InputReader strategy which provides the implementation
/// details for different inputs such as file (`ifs_input_reader`)
/// and raw buffer bytes (`str_input_reader`).
///

template <typename InputReaderConcrete>
    requires is_input_reader_v<InputReaderConcrete>
class token_citerator final {
    using input_reader_type = InputReaderConcrete;

public:
    ///
    /// Special member functions
    ///

    explicit token_citerator(const input_reader_type& ir)
        try : m_input_reader { ir }
            , m_current_location { ir.tell() }
    {
        // Safety: This is guaranteed by the input reader strategies ctors.
        assert(m_input_reader.ready());

        // This means that the input is empty. The operator*() calls consume() if it
        // has not been called yet, meaning that dereferencing an empty input will
        // force-read a token - behaviour that we do not want.
        if (m_input_reader.eof())
            m_consumed_first = true;
    } catch (const input_reader_exception &ire) {
        throw token_exception{ire.what()};
    }

    token_citerator(const token_citerator& rhs)
        try : m_input_reader { rhs.m_input_reader }
        , m_current_location { rhs.m_current_location }
        , m_consumed_first { rhs.m_consumed_first }
        , m_consumed { rhs.m_consumed ? rhs.m_consumed->clone() : nullptr }
    {
        // Safety: This isguaranteed by the input reader strategies ctors.
        assert(m_input_reader.ready());
    } catch (const input_reader_exception &ire) {
        throw token_exception{ire.what()};
    }

    token_citerator& operator=(const token_citerator& rhs)
    {
        m_input_reader = rhs.m_input_reader;
        m_consumed_first = rhs.m_consumed_first;
        m_consumed = rhs.m_consumed ? rhs.m_consumed->clone() : nullptr ;
        m_current_location = rhs.m_current_location;
        return *this;
    }

    token_citerator(token_citerator&&) noexcept = default;
    token_citerator& operator=(token_citerator&&) noexcept = default;

    ~token_citerator() noexcept = default;

    [[nodiscard]] bool operator==(const token_citerator &rhs) const noexcept {
        return m_current_location == rhs.m_current_location;
    }
    [[nodiscard]] std::strong_ordering operator<=>(const token_citerator &rhs) const noexcept {
        return m_current_location <=> rhs.m_current_location;
    }

    ///
    /// Iterator behaviour
    ///

    // Throws token_exception if the current token is consumed or if the entire token stream is empty.
    [[nodiscard]] mystd::unique_ptr<token> operator*() {
        if (!m_consumed && !m_consumed_first){
            consume_and_store();
            m_consumed_first = true;
        }

        if (!m_consumed)
            throw token_exception_here("Trying to access consumed token.");

        auto consumed = std::move(m_consumed);
        m_consumed.reset();
        return consumed;
    }

    [[nodiscard]] const token *peek_unsafe() {
        if (!m_consumed_first){
            consume_and_store();
            m_consumed_first = true;
        }

        return m_consumed.get();
    }

    // Once the token stream is consumed no action is performed. However, if the "new" item is accessed
    // after the end, the an exception is thrown.
    token_citerator& operator++() {
        consume_and_store();
        return *this;
    }

    token_citerator operator++(int) {
        auto copy {*this};
        consume_and_store();
        return copy;
    }

private:

    ///
    /// Tokenization process
    /// The `consume_*` family of functions do the actual work of preparing the acquired input
    /// from the `input_reader` strategy to tokens - values of the `token` hierarchy.
    /// `consume_and_store()` is the driver function - it calls `consume()` and stores its result
    /// in `m_consumed` which stores "the current token".
    ///
    void consume_and_store() {
        consume_whitespace();
        if (has_more())
            m_consumed = consume();
    }

    mystd::unique_ptr<token> consume() {
        consume_whitespace();

        switch ([[maybe_unused]] char sym = peek()) {
        case '-': [[fallthrough]];
        case '0' ... '9':
            return consume_number();
        case '"':
            return consume_string();
        case 'a' ... 'z':
            return consume_keyword();
        default:
            return consume_punct();
        }

        mystd::unreachable();
    }

    mystd::unique_ptr<token> consume_number() {
        std::string value;
        auto valid_in_number = [](char sym) -> bool {
            return std::isdigit(sym)
                   || sym == '.'
                   || sym == '-'
                   || sym == '+'
                   || sym == 'e' || sym == 'E';
        };

        for (char sym = peek(); valid_in_number(sym); sym = peek()){
            value += get();
            if (!has_more())
                break;
        }

        return make_token<token_number>(std::atof(value.c_str()));
    }

    mystd::unique_ptr<token> consume_string() {
        std::string value;
        expect_symbol('"'); // Strings always begin this way.
        while (peek() != '"'){
            value += get();
            if (value.back() == '\\') {
                switch (char sym = get()) {
                    // clang-format off
                    break; case 'n': value.back() = '\n';
                    break; case 'r': value.back() = '\r';
                    break; case 't': value.back() = '\t';
                    break; default: value += sym;
                    // clang-format on
                }
            }
        }
        expect_symbol('"');

        return make_token<token_string>(value);
    }

    mystd::unique_ptr<token> consume_keyword() {
        const static std::string literal_true = "true";
        const static std::string literal_false = "false";
        const static std::string literal_null = "null";

        std::string value;
        while (std::isalpha(peek()))
            value += get();

        using enum token_keyword::kind;
        if (value == literal_true)
            return make_token<token_keyword>(True);
        if (value == literal_false)
            return make_token<token_keyword>(False);
        if (value == literal_null)
            return make_token<token_keyword>(Null);

        unexpected_symbol(value.back());
    }

    mystd::unique_ptr<token> consume_punct(){
        const char sym = get();
        if (token_punct::is_valid(sym))
            return make_token<token_punct>(sym);
        unexpected_symbol(sym);
    }

    void consume_whitespace() {
        while (has_more() && std::isspace(peek())) {
            char sym = get(get_preference::DontUpdateLocation);
            // clang-format off
            switch (sym) {
                break; case '\n':
                m_current_location.column_num() = 0;
                ++m_current_location.line_num();
                break; case '\t':
                m_current_location.column_num() += 4;
                ++m_current_location.line_num();
                break; case '\r':
                m_current_location.column_num() = 0;
                break; case ' ': [[fallthrough]];
            default:   // eof
                ++m_current_location.column_num();
            }
            // clang-format on
        }
    }

    char peek() const {
        try {
            return m_input_reader.peek();
        } catch (const input_reader_exception &ire) {
            throw token_exception_here(ire.what());
        }
    }

    enum class get_preference {
        DontUpdateLocation,
        UpdateLocation
    };

    char get(get_preference preference = get_preference::UpdateLocation) {
        char sym;
        try {
            sym = m_input_reader.get();
            m_current_location.detail_pos() = m_input_reader.tell();
        } catch (input_reader_exception &ire) {
            throw token_exception_here(ire.what());
        }
        if (preference == get_preference::UpdateLocation)
            ++m_current_location.column_num();
        return sym;
    }

    ///
    /// Error reporting helpers.
    /// Most of them are private as they are only helpful during the tokenization process,
    /// but there are some which are using by the parser.
private:
    void expect_symbol(char sym) {
        if (const char next_sym = get(); next_sym != sym) {
            std::string msg = m_current_location.to_string() + ": Expected '" + sym + "' but got '" + next_sym + "' instead.";
            throw token_exception_here(std::move(msg));
        }
    }

    [[noreturn]] void unexpected_symbol(char sym) {
        std::string msg = m_current_location.to_string() + ": Unexpected symbol '" + sym + "' found.";
        throw token_exception_here(std::move(msg));
    }

    template <typename T>
    [[nodiscard]] token_exception token_exception_here(T&& msg) const {
        return token_exception{std::forward<T>(msg), m_current_location};
    }

public:
    [[nodiscard]] bool has_more() const noexcept { return !m_input_reader.eof(); }

    void expect_has_more() const {
        if (!has_more()) {
            std::string msg = m_current_location.to_string() + ": Trying to consume after end.";
            throw token_exception_here(std::move(msg));
        }
    }

public:
    ///
    /// Properties
    ///

    [[nodiscard]] const input_reader_type &input_reader() const & {
        return m_input_reader;
    }

    [[nodiscard]] const location &current_location() const & {
        return m_current_location;
    }

    [[nodiscard]] location current_location() && {
        return std::move(m_current_location);
    }

private:
    input_reader_type m_input_reader;

    location m_current_location;

    bool m_consumed_first{false};
    mystd::unique_ptr<token> m_consumed;
};

///
/// The `tokenizer` class.
/// This is a helper class which seems to not be of much use :D.
/// I primarily wanted it for testing purposes in order to have a
/// type which is reponsible for creating begin and end token iterators.
/// The `tokenizer` type is generic over the input reader strategy and
/// passess is directly to the `token_citerator`s that it creates.
///

template <typename InputReaderConcrete>
    requires is_input_reader_v<InputReaderConcrete>
class tokenizer {
    using input_reader_type = InputReaderConcrete;

public:
    explicit tokenizer(const input_reader_type &ir)
        : m_input_reader { ir } { }

    explicit tokenizer(input_reader_type&& ir)
        : m_input_reader { std::move(ir) } { }

public:
    using token_iterator_type = token_citerator<input_reader_type>;

    token_iterator_type begin() const {
        return token_iterator_type { m_input_reader.begin() };
    }

    token_iterator_type end() const {
        return token_iterator_type { m_input_reader.end() };
    }

public:
    [[nodiscard]] const input_reader_type& input_reader() const&
	{
        return m_input_reader;
    }

private:
    input_reader_type m_input_reader;
};

/// Aliases
using ifs_tokenizer = tokenizer<ifs_input_reader>;
using str_tokenizer = tokenizer<str_input_reader>;

} // namespace json_parser

#endif // FMI_JSON_PARSER_TOKENIZER_INCLUDED
