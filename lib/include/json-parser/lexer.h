#ifndef FMI_JSON_PARSER_INPUT_READER_INCLUDED
#define FMI_JSON_PARSER_INPUT_READER_INCLUDED

#ifndef FMI_ADHERE_TO_TASK
#include <iterator>
#endif

// #include <json-parser/input_reader.h>

namespace json_parser {

using input_reader = std::ifstream;

/// JSON Lexer
/// Implemented as an input operator (according to the STL nomenclature).
class lexer {
	struct location {
		std::size_t line_num;
		std::size_t column_num;
	};

	enum class token {
		// "Keywords"
		kw_null,
		kw_true,
		kw_false,

		// Punctuators
		punc_column,
		punc_comma,
		punc_lbracket,
		punc_rbracket,
		punc_lbrace,
		punc_rbrace,

		// Literals
		lit_string,
		lit_integer,
		lit_fraction,
		lit_exponent,

		whitespace,
	};

public:

	// explicit lexer(const input_reader &ir)
	// 	: m_reader{ir} {}
	// lexer(const lexer &) = default;
	// lexer& operator=(const lexer &) = default;

private:
	input_reader m_reader;
	location m_location;
};

#ifndef FMI_ADHERE_TO_TASK
// static_assert(std::input_iterator<lexer>, "JSON Lexer is not an input iterator");
#endif

}

#endif // FMI_JSON_PARSER_INPUT_READER_INCLUDED
