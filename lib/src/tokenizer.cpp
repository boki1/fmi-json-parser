#include <json-parser/tokenizer.h>

namespace json_parser {

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
