#include <json-parser/parser.h>
#include <json-parser/tokenizer.h>

namespace json_parser {

const json &parser::parse() & {
    if (!m_parsed)
        parse_and_store();
    return *m_parsed;
}

json parser::parse() && {
    if (!m_parsed)
        parse_and_store();
    auto taken = std::move(*m_parsed);
    m_parsed.reset();
    return taken;
}

void parser::parse_and_store() {
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

json::pmrvalue parser::parse_object() {
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

json::pmrvalue parser::parse_array() {
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

json::pmrvalue parser::parse_value() {
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

}
