#ifndef FMI_JSON_PARSER_TOKENIZER_INCLUDED
#define FMI_JSON_PARSER_TOKENIZER_INCLUDED

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include <mystd/memory.h>
#include <mystd/optional.h>
#include <mystd/utility.h>

namespace json_parser {

class location {

public:
	explicit location(std::fstream::pos_type stream_pos)
		: m_stream_pos { stream_pos }
	{
	}

	[[nodiscard]] bool operator<(const location& rhs) const noexcept
	{
		return m_stream_pos < rhs.m_stream_pos;
	}

	[[nodiscard]] std::string to_string() const
	{
		std::string s;
		s += std::to_string(m_line_num);
		s += ':';
		s += std::to_string(m_column_num);
		s += " (";
		s += std::to_string(m_stream_pos);
		s += ")";
		return s;
	}

	[[nodiscard]] auto line_num() const& noexcept { return m_line_num; }
	[[nodiscard]] auto column_num() const& noexcept { return m_column_num; }
	[[nodiscard]] auto stream_pos() const& noexcept { return m_stream_pos; }

	[[nodiscard]] auto& line_num() & noexcept { return m_line_num; }
	[[nodiscard]] auto& column_num() & noexcept { return m_column_num; }
	[[nodiscard]] auto& stream_pos() & noexcept { return m_stream_pos; }

private:
	std::size_t m_line_num { 0 };
	std::size_t m_column_num { 0 };
	std::fstream::pos_type m_stream_pos;
};

class token_exception : public std::exception {

public:
	explicit token_exception(std::string msg,
		mystd::optional<location> location = {})
		: m_location { location }
		, m_msg { std::move(msg) }
	{
    }

	[[nodiscard]] const char* what() const noexcept { return m_msg.c_str(); }

private:
	mystd::optional<location> m_location;
	std::string m_msg;
};

struct token {
	virtual ~token() noexcept = default;

    virtual void serialize(std::ostream &os) const = 0;
    virtual std::unique_ptr<token> clone() const noexcept = 0;
};

class token_string : public token {
public:
	explicit token_string(const std::string& value)
		: m_value { value }
	{
    }

	[[nodiscard]] const std::string& value() const noexcept { return m_value; }

    void serialize(std::ostream &os) const override;

    std::unique_ptr<token> clone() const noexcept override { return std::make_unique<token_string>(*this); }

private:
	std::string m_value;
};

class token_number : public token {
public:
	explicit token_number(double value)
		: m_value { value }
	{
    }

	[[nodiscard]] double value() const noexcept { return m_value; }

    void serialize(std::ostream &os) const override;

    std::unique_ptr<token> clone() const noexcept override { return std::make_unique<token_number>(*this); }

private:
	double m_value;
};

class token_keyword : public token {
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

    [[nodiscard]] kind value() const noexcept { return m_value; }

    void serialize(std::ostream &os) const override;

    std::unique_ptr<token> clone() const noexcept override { return std::make_unique<token_keyword>(*this); }

private:
    kind m_value;
};

class token_punct : public token {
public:
	explicit token_punct(char value)
		: m_value { value }
	{
		// Safety: the token_punct ctor is called only with valid data!
		assert(token_punct::is_valid(value));
    }

	[[nodiscard]] char value() const noexcept { return m_value; }

    void serialize(std::ostream& os) const override;

    std::unique_ptr<token> clone() const noexcept override { return std::make_unique<token_punct>(*this); }

    [[nodiscard]] static bool is_valid(char value);

private:
	char m_value;
};

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
/// \brief The token_citerator class
/// \todo Abstract away the input stream type. Enable tokenizing strings.
class token_citerator {
	friend class tokenizer;

public:
    token_citerator(const std::string& input_filename, std::ios_base::seekdir input_seekdir);

    token_citerator(const token_citerator& rhs);
	token_citerator& operator=(const token_citerator& rhs);

    token_citerator(token_citerator&&) noexcept = default;
    token_citerator& operator=(token_citerator&&) noexcept = default;

    ~token_citerator() noexcept = default;

	[[nodiscard]] bool operator==(const token_citerator& rhs) const;
	[[nodiscard]] bool operator!=(const token_citerator& rhs) const;
	[[nodiscard]] bool operator<(const token_citerator& rhs) const;
	[[nodiscard]] bool operator>(const token_citerator& rhs) const;
	[[nodiscard]] bool operator<=(const token_citerator& rhs) const;
	[[nodiscard]] bool operator>=(const token_citerator& rhs) const;

    // Throws token_exception if the current token is consumed or if the entire token stream is empty.
	[[nodiscard]] mystd::unique_ptr<token> operator*();

    // Once the token stream is consumed no action is performed. However, if the "new" item is accessed
    // after the end, the an exception is thrown.
	token_citerator& operator++();
	token_citerator operator++(int);

    [[nodiscard]] const token *peek_unsafe() {
        if (!m_consumed_first){
            consume_and_store();
            m_consumed_first = true;
        }

        return m_consumed.get();
    }

    // Rationale:
    // Changing the input filename would cause inconsistencies among
    // any existing iterators of the tokenizer instance.
	void set_input_filename(const std::string&) = delete;

	[[nodiscard]] const std::string& input_filename() const& { return m_input_filename; }
	[[nodiscard]] const location& current_location() const& { return m_current_location; }
	[[nodiscard]] std::string input_filename() && { return std::move(m_input_filename); }
	[[nodiscard]] location current_location() && { return std::move(m_current_location); }

private:
    void consume_and_store();
    mystd::unique_ptr<token> consume();
    mystd::unique_ptr<token> consume_number();
	mystd::unique_ptr<token> consume_string();
	mystd::unique_ptr<token> consume_keyword();
	mystd::unique_ptr<token> consume_punct();
	void consume_whitespace();

	char peek() noexcept { return m_input.peek(); }

	enum class location_update_preference { DontUpdate,
		DoUpdate };
	char get(location_update_preference p = location_update_preference::DoUpdate);

    void expect_symbol(char sym);
	[[noreturn]] void unexpected_symbol(char sym);

    template <typename T>
    [[nodiscard]] token_exception token_exception_here(T&& msg) const {
        return token_exception{std::forward<T>(msg), m_current_location};
    }

public:
    bool has_more() const noexcept { return !m_input.eof(); }
    void expect_has_more() const;

private:
	std::string m_input_filename;
	std::ifstream m_input;
	location m_current_location;
    bool m_consumed_first{false};
    mystd::unique_ptr<token> m_consumed;
};

class tokenizer {
public:
	explicit tokenizer(const std::string& input_filename)
		: m_input_filename { input_filename }
	{
    }

	// There is not actual difference between the iteratos constructed on const
	// and non-const tokenizer instances - they are both const.
	token_citerator begin() const;
	token_citerator end() const;

public:
	[[nodiscard]] const std::string& input_filename() const&
	{
		return m_input_filename;
	}

	[[nodiscard]] std::string input_filename() &&
	{
		return std::move(m_input_filename);
	}

private:
	std::string m_input_filename;
};

} // namespace json_parser

#endif // FMI_JSON_PARSER_TOKENIZER_INCLUDED
