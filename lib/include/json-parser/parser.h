#ifndef FMI_JSON_PARSER_PARSER_INCLUDED
#define FMI_JSON_PARSER_PARSER_INCLUDED

#include <mystd/optional.h>

#include <json-parser/tokenizer.h>
#include <json-parser/json.h>
#include <json-parser/input_reader.h>

namespace json_parser {

///
/// The `parser_exception` class.
/// This type is used for reporting any errors that occur during the
/// whole parsing process. Any exceptions thrown during tokenization or
/// acquiring input are caught and rethrown as instances of `parser_exception`
/// containing the initial error reporting message.
///
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

///
/// The `parser` class.
/// This type implements the logic of the parsing process. It uses the
/// provided `tokenizer` type for the implementation of token acquiring.
/// Going token by token the syntactic structures of the JSON type are
/// formed. They are stored in-memory as an instance of `json` type.
///

template <typename InputReaderConcrete>
    requires is_input_reader_v<InputReaderConcrete>
class parser final {
    using input_reader_type = InputReaderConcrete;
    using tokenizer_type = tokenizer<input_reader_type>;
    using token_iterator_type = typename tokenizer_type::token_iterator_type;

public:
    ///
    /// Special member functions.
    ///

    explicit parser(input_reader_type&& ir)
        try : m_tokenizer{std::move(ir)}
            , m_token_cit{m_tokenizer.begin()}
    { } catch (const token_exception &te) {
        throw parser_exception(te.what());
    }

    explicit parser(const input_reader_type& ir)
        try : m_tokenizer{ir}
            , m_token_cit{m_tokenizer.begin()}
    { } catch (const token_exception &te) {
        throw parser_exception(te.what());
    }

public:
    ///
    /// Parsing behaviour.
    ///
    /// The parsing process is started by the `parse()` function or the `operator()`.
    /// The dirty work is performed in the `parse_*()` family of functions. They are
    /// private as they aren't of big interest to the end-user.
    ///

    const json &parse() & {
        if (!m_parsed)
            parse_and_store();
        return *m_parsed;
    }

    const json &operator()() & { return parse(); }

    json parse() && {
        if (!m_parsed)
            parse_and_store();
        auto taken = std::move(*m_parsed);
        m_parsed.reset();
        return taken;
    }

    json operator()() && { return parse(); }

private:

    void parse_and_store() {
        const auto token_end = m_tokenizer.end();

        if (m_token_cit == token_end) {
            m_parsed.emplace();
            return;
        }

        try {
            auto root_node = parse_value();
            m_parsed.emplace(json{std::move(root_node)});
        } catch (const token_exception &t) {
            // The tokenizer is too low-level to be reasonable
            // to report errors using its name :).
            throw parser_exception(t.what());
        }
    }

    [[nodiscard]] json::pmrvalue parse_object() {
        mystd::unique_ptr<token> object_begin = expect_token<token_punct>();
        // Safety: parse_object() is called only when '{' is found.
        // Also, on entering this function the '{' token is not yet consumed.
        assert(token_as<token_punct>(object_begin)->value() == '{');

        json::pmrvalue node = make_node<json::object>();
        json::object &node_as_object = dynamic_cast<json::object &>(*node.get());

        for (;;) {
            json::string key_as_str = [&]() {
                json::pmrvalue key = parse_value();
                auto *key_as_str_ptr = dynamic_cast<json::string *>(key.get());
                if (!key_as_str_ptr)
                    throw parser_exception_here("Expected string as key in JSON object");
                return *key_as_str_ptr;
            }();

            mystd::unique_ptr<token> column = expect_token<token_punct>();
            if (token_as<token_punct>(column)->value() != ':')
                throw parser_exception_here("Expected ':' after key in JSON object.");

            json::pmrvalue val = parse_value();
            node_as_object.append(std::move(key_as_str), std::move(val));

            mystd::unique_ptr<token> delimiter = expect_token<token_punct>();
            const char delimiter_sym = token_as<token_punct>(delimiter)->value();
            if (delimiter_sym == '}')
                break;
            if (delimiter_sym != ',')
                throw parser_exception_here("Expected either '}' or ',' after key-value pair in JSON object.");
        }

        return node;
    }

    [[nodiscard]] json::pmrvalue parse_array() {
        mystd::unique_ptr<token> array_begin = expect_token<token_punct>();
        // Safety: parse_array() is called only when '[' is found.
        // Also, on entering this function the '[' token is not yet consumed.
        assert(token_as<token_punct>(array_begin)->value() == '[');

        json::pmrvalue node = make_node<json::array>();
        json::array &node_as_array = dynamic_cast<json::array &>(*node.get());
        for (;;) {
            node_as_array.append(parse_value());
            mystd::unique_ptr<token> delimiter = expect_token<token_punct>();
            const char delimiter_sym = token_as<token_punct>(delimiter)->value();
            if (delimiter_sym == ']')
                break;
            if (delimiter_sym != ',')
                throw parser_exception_here("Expected either ']' or ',' after value in JSON array.");
        }

        return node;
    }

    [[nodiscard]] json::pmrvalue parse_value() {
        // Safety: If parse_value() was called then there has to be a value, otherwise it
        // wouldn't be called in the first place. That is guaranteed by the called.
        expect_has_more();

        // Try parsing an array or an object.
        // The token in the iterator should not be consumed here, because that would
        // invalidate the expected "grammar" in parse_array() and parse_object().
        // Safety:
        const token *next_tok_ptr = m_token_cit.peek_unsafe();
        assert(next_tok_ptr != nullptr);

        if (auto *punct = token_as<token_punct>(next_tok_ptr); punct != nullptr) {
            switch (punct->value()) {
            case '[': return parse_array();
            case '{': return parse_object();
            }

            std::string msg = "Expected valid JSON value, but got an unexpected punctuator - '";
            msg += punct->value();
            msg += "'";
            throw parser_exception_here(std::move(msg));
        }

        // Try parsing a "trivial" value.
        // Here we can safely consume the next token as it is the only one.
        auto next_tok = *m_token_cit;
        next_tok_ptr = next_tok.get();

        json::pmrvalue next_node;
        if (auto *str = token_as<token_string>(next_tok_ptr); str != nullptr)
            next_node = make_node<json::string>(*str);
        else if (auto *num = token_as<token_number>(next_tok_ptr); num != nullptr)
            next_node = make_node<json::number>(*num);
        else if (auto *kw = token_as<token_keyword>(next_tok_ptr); kw != nullptr){
            if (kw->value() == token_keyword::kind::Null)
                next_node = make_node<json::null>();
            else
                next_node = make_node<json::boolean>(*kw);
        }

        if (next_node) {
            ++m_token_cit;
            return next_node;
        }

        // We tried parsing whatever we could. Now fail successfully.
        throw parser_exception_here("Expected valid JSON value, but no such was found.");
    }

private:
    ///
    /// Error handling and reporting helpers.
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

public:
    ///
    /// Properties.
    ///
    /// The `parse()` function implements the most import property access - the `m_parsed`
    /// member is returned from it, but not before it is populated with the JSON that is
    /// acquired from the `input_reader` strategy.
    ///

    [[nodiscard]] static const std::string &input_reader_kind() noexcept {
        return typename input_reader_type::kind();
    }

private:
    tokenizer_type m_tokenizer;
    token_iterator_type m_token_cit;
	mystd::optional<json> m_parsed;
};

/// Aliases
using ifs_parser = parser<ifs_input_reader>;
using str_parser = parser<str_input_reader>;

}

#endif // FMI_JSON_PARSER_PARSER_INCLUDED
