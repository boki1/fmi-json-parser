#include <json-parser/tokenizer.h>

namespace json_parser {

token_citerator::token_citerator(const std::string& input_filename, std::ios_base::seekdir input_seekdir)
    : m_input_filename { input_filename }
    , m_input { input_filename }
    , m_current_location { m_input.seekg(0, input_seekdir).tellg() }
{
    if (!m_input.is_open() || !m_input.good())
        throw token_exception{"token_citerator: Cannot open file '" + m_input_filename + "'!"};

    if (input_seekdir == std::ios_base::end)
        m_consumed_first = true;
}

token_citerator::token_citerator(const token_citerator& rhs)
    : m_input_filename { rhs.m_input_filename }
    , m_input { m_input_filename }
    , m_current_location { rhs.m_current_location }
    , m_consumed_first { rhs.m_consumed_first }
    , m_consumed { rhs.m_consumed->clone() }
{
    if (!m_input.is_open() || !m_input.good())
        throw token_exception{"token_citerator: Cannot open file '" + m_input_filename + "'!"};
}

token_citerator& token_citerator::operator=(const token_citerator& rhs)
{
    m_input_filename = rhs.m_input_filename;
	m_input = std::ifstream { rhs.m_input_filename };
    m_consumed_first = rhs.m_consumed_first;
    m_consumed = rhs.m_consumed->clone();
    m_current_location = rhs.m_current_location;
	return *this;
}

[[nodiscard]] bool token_citerator::operator<(const token_citerator& rhs) const { return m_current_location < rhs.m_current_location; }
[[nodiscard]] bool token_citerator::operator>(const token_citerator& rhs) const { return rhs < *this; }
[[nodiscard]] bool token_citerator::operator<=(const token_citerator& rhs) const { return *this < rhs || *this == rhs; }
[[nodiscard]] bool token_citerator::operator>=(const token_citerator& rhs) const { return *this > rhs || *this == rhs; }
[[nodiscard]] bool token_citerator::operator==(const token_citerator& rhs) const { return !(*this > rhs) && !(*this < rhs); }
[[nodiscard]] bool token_citerator::operator!=(const token_citerator& rhs) const { return !(*this == rhs); }

[[nodiscard]] mystd::unique_ptr<token> token_citerator::operator*() {
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

token_citerator& token_citerator::operator++() {
    consume_and_store();
	return *this;
}

token_citerator token_citerator::operator++(int) {
	auto copy {*this};
    consume_and_store();
	return copy;
}

[[nodiscard]] char token_citerator::get(location_update_preference update_preference)
{
	char sym;
	try {
		sym = m_input.get();
		m_current_location.stream_pos() = m_input.tellg();
	} catch (std::ios::failure&) {
        throw token_exception_here("Unexpected failure of input stream.");
	}
	using enum location_update_preference;
	if (update_preference == DoUpdate)
		++m_current_location.column_num();
	return sym;
}

void token_citerator::expect_has_more() const
{
    if (!has_more()) {
        std::string msg = m_current_location.to_string() + ": Trying to consume after end.";
        throw token_exception_here(std::move(msg));
    }
}

void token_citerator::expect_symbol(char sym)
{
	if (const char next_sym = get(); next_sym != sym) {
        std::string msg = m_current_location.to_string() + ": Expected '" + sym + "' but got '" + next_sym + "' instead.";
        throw token_exception_here(std::move(msg));
    }
}

void token_citerator::unexpected_symbol(char sym)
{
    std::string msg = m_current_location.to_string() + ": Unexpected symbol '" + sym + "' found.";
    throw token_exception_here(std::move(msg));
}

void token_citerator::consume_whitespace()
{
    while (has_more() && std::isspace(peek())) {
        char sym = get(location_update_preference::DontUpdate);
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

void token_citerator::consume_and_store() {
    consume_whitespace();
    if (has_more())
        m_consumed = consume();
}

mystd::unique_ptr<token> token_citerator::consume()
{
	consume_whitespace();

    switch ([[maybe_unused]] char sym = peek()) {
    case '-': [[fallthrough]];
    case '1' ... '9':
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

mystd::unique_ptr<token> token_citerator::consume_punct()
{
	const char sym = get();
	if (token_punct::is_valid(sym))
		return make_token<token_punct>(sym);
	unexpected_symbol(sym);
}

mystd::unique_ptr<token> token_citerator::consume_number()
{
    std::string value;
    auto valid_in_number = [](char sym) -> bool {
        return std::isdigit(sym)
               || sym == '.'
               || sym == '-'
               || sym == '+'
               || sym == 'e' || sym == 'E';
    };

    for (char sym = peek(); valid_in_number(sym); sym = peek())
        value += get();

    return make_token<token_number>(std::atof(value.c_str()));
}

// TODO: This seems way too naive to me to work fine.
// Test this very thoroughly.
mystd::unique_ptr<token> token_citerator::consume_string()
{
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

mystd::unique_ptr<token> token_citerator::consume_keyword()
{
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

token_citerator tokenizer::begin() const
{
	return token_citerator { m_input_filename, std::ios_base::beg };
}

token_citerator tokenizer::end() const
{
	return token_citerator { m_input_filename, std::ios_base::end };
}

void token_string::serialize(std::ostream& os) const
{
    os << '"' << m_value << '"';
}

void token_number::serialize(std::ostream& os) const
{
    os << m_value;
}

void token_keyword::serialize(std::ostream& os) const
{
    using enum kind;
	switch (m_value) {
		break; case True: os << "true";
		break; case False: os << "false";
		break; case Null: os << "null";
    }
}

void token_punct::serialize(std::ostream& os) const
{
    os << m_value;
}

[[nodiscard]] bool token_punct::is_valid(char value)
{
    static const char valid[] = "{}[]:,";
    for (const char *pbegin = valid, *pend = valid + sizeof(valid);
         pbegin < pend; ++pbegin)
        if (*pbegin == value)
            return true;
    return false;
}

}
