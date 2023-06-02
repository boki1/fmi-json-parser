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

	token_exception(const token_exception&) = default;
	token_exception& operator=(const token_exception&) = default;

	[[nodiscard]] const char* what() const noexcept { return m_msg.c_str(); }

private:
	mystd::optional<location> m_location;
	std::string m_msg;
};

struct token {
	virtual ~token() noexcept = default;
	virtual std::ostream& operator<<(std::ostream& os) const noexcept = 0;
};

class token_string : public token {
public:
	explicit token_string(const std::string& value)
		: m_value { value }
	{
	}
	token_string(const token_string&) = default;
	token_string& operator=(const token_string&) = default;

	[[nodiscard]] const std::string& value() const noexcept { return m_value; }

	std::ostream& operator<<(std::ostream& os) const noexcept override;

private:
	std::string m_value;
};

class token_number : public token {
public:
	explicit token_number(double value)
		: m_value { value }
	{
	}
	token_number(const token_number&) = default;
	token_number& operator=(const token_number&) = default;

	[[nodiscard]] double value() const noexcept { return m_value; }

	std::ostream& operator<<(std::ostream& os) const noexcept override;

private:
	double m_value;
};

class token_keyword : public token {
public:
	enum class keyword { True,
		False,
		Null };

	explicit token_keyword(keyword value)
		: m_value { value }
	{
	}
	token_keyword(const token_keyword&) = default;
	token_keyword& operator=(const token_keyword&) = default;

	[[nodiscard]] keyword value() const noexcept { return m_value; }

	std::ostream& operator<<(std::ostream& os) const noexcept override;

private:
	keyword m_value;
};

class token_punct : public token {
public:
	explicit token_punct(char value)
		: m_value { value }
	{
		// Safety: the token_punct ctor is called only with valid data!
		assert(token_punct::is_valid(value));
	}

	token_punct(const token_punct&) = default;
	token_punct& operator=(const token_punct&) = default;

	[[nodiscard]] char value() const noexcept { return m_value; }

	std::ostream& operator<<(std::ostream& os) const noexcept override;

	[[nodiscard]] static bool is_valid(char value)
	{
		static const char valid[] = "{}[]:,";
		for (const char *pbegin = valid, *pend = valid + sizeof(valid);
			 pbegin < pend; ++pbegin)
			if (*pbegin == value)
				return true;
		return false;
	}

private:
	char m_value;
};

template <typename Kind, typename Value = Kind>
mystd::unique_ptr<Kind> make_token(Value&& value)
{
	return mystd::make_unique<Kind>(mystd::forward<Value>(value));
}

///
/// \brief The token_citerator class
///
class token_citerator {
	friend class tokenizer;

public:
	token_citerator(const std::string& input_filename, std::ios_base::seekdir input_seekdir)
		: m_input_filename { input_filename }
		, m_input { input_filename }
		, m_current_location { m_input.seekg(0, input_seekdir).tellg() }
	{
		if (!m_input.is_open() || !m_input.good())
			throw token_exception{"token_citerator: Cannot open file '" + m_input_filename + "'!"};
	}

	token_citerator(const token_citerator& rhs)
		: m_input_filename { rhs.m_input_filename }
		, m_input { m_input_filename }
		, m_current_location { rhs.m_current_location }
	{
		if (!m_input.is_open() || !m_input.good())
			throw token_exception{"token_citerator: Cannot open file '" + m_input_filename + "'!"};
	}

	token_citerator& operator=(const token_citerator& rhs);

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

    bool has_more() const noexcept { return !m_input.eof(); }
    void expect_has_more() const;

    void expect_symbol(char sym);
	[[noreturn]] void unexpected_symbol(char sym);

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

	tokenizer(const tokenizer&) = default;
	tokenizer& operator=(const tokenizer&) = default;

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
