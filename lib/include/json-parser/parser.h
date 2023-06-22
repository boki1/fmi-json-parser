#ifndef FMI_JSON_PARSER_PARSER_INCLUDED
#define FMI_JSON_PARSER_PARSER_INCLUDED

#include <mystd/optional.h>

#include <json-parser/tokenizer.h>
#include <json-parser/json.h>

namespace json_parser {

class parser_exception : public std::exception {

public:
    explicit parser_exception(std::string msg,
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

class parser {

public:
    explicit parser(const std::string& filename)
        try : m_tokenizer{filename}
        , m_token_cit{m_tokenizer.begin()}
    {
    } catch (const token_exception &te) {
        throw parser_exception(te.what());
    }

    const json &parse() &;
    const json &operator()() & { return parse(); }

    json parse() &&;
    json operator()() && { return parse(); }

private:
    ///
    /// Parsing behaviour
    ///

    void parse_and_store();
    [[nodiscard]] json::pmrvalue parse_object();
    [[nodiscard]] json::pmrvalue parse_array();
    [[nodiscard]] json::pmrvalue parse_value();

    ///
    /// Error handling and helpers
    ///

    [[nodiscard]] bool has_more() const noexcept { return m_token_cit.has_more(); }

    void expect_has_more() const {
        if (!m_token_cit.has_more())
            throw parser_exception_here("Expected more tokens during parsing.");
    }

    template <typename TokenKind>
    mystd::unique_ptr<token> expect_token() {
        expect_has_more();
        mystd::unique_ptr<token> next_token = *m_token_cit++;
        if (!token_as<TokenKind>(next_token))
            throw parser_exception_here(std::string("Expected token of type `") + typeid(TokenKind).name() + "` but no such was found.");
        return next_token;
    }

    template <typename T>
    [[nodiscard]] parser_exception parser_exception_here(T&& msg) const {
        return parser_exception(mystd::forward<T>(msg), m_token_cit.current_location());
    }

private:
    tokenizer m_tokenizer;
    token_citerator m_token_cit;
	mystd::optional<json> m_parsed;
};

}

#endif // FMI_JSON_PARSER_PARSER_INCLUDED
