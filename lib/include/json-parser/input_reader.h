#ifndef FMI_JSON_PARSER_INPUT_READER_INCLUDED
#define FMI_JSON_PARSER_INPUT_READER_INCLUDED

#include <fstream>

namespace json_parser {

/// This type serves the lexer with the input read from a JSON file.
/// I would very much like this to use memory-mapped files in order
/// to make this faster. However, I will begin with the current strategy
/// and improve upon it later.
using input_reader = std::ifstream;

}

#endif // FMI_JSON_PARSER_INPUT_READER_INCLUDED
