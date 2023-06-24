#include <json-editor/editor.h>

namespace json_editor {

void editor::add_cmd(const std::string &cmd_name, cmd_type func) {
    m_commands.emplace(cmd_name, func);
}

void editor::remove_cmd(const std::string &cmd_name) {
    m_commands.erase(cmd_name);
}

bool editor::perform_cmd() {
    m_out << prompt;
    std::string cmd_name;
    m_in >> cmd_name;
    if (!m_commands.contains(cmd_name)) {
        m_out << "json-editor: Unknown command '" + cmd_name + "'.\n";
        return false;
    }

    try {
        cmd_type cmd = m_commands.at(cmd_name);
        return (*cmd)(*this);
    } catch (const json_parser::parser_exception &pe) {
        m_out << "Error: " << pe.what() << '\n';
    }

    return false;
}

void editor::loop() {
    while (!perform_cmd())
        ;
}

} // namespace json_editor

namespace json_editor::commands {

using json_editor::editor;
using namespace json_parser;

///
/// Helper functions used in the commands implementation.
///

static std::string read_string_from(std::istream &is) {
    std::string read;
    while (true) {
        std::string piece;
        is >> piece;
        read += piece;
        if (!read.starts_with("\""))
            break;
        if (read.ends_with("\""))
            break;
    }

    return read;
}

static std::string read_snippet_from(std::istream &is) {
    std::string read;
    while (true) {
        std::string piece;
        is >> piece;
        if (read.size() > 0 && piece == "```")
            break;
        if (read.size() == 0 && piece == "```")
            continue;
        read += piece;
    }

    return read;
}

static json_parser::json::pmrvalue string_to_trivial_json(const std::string &contents) {
    json_parser::str_parser parser{json_parser::str_input_reader{contents}};
    json_parser::json parsed = parser();
    if (parsed.trivial())
        return parsed.take();
    return nullptr;
}

static mystd::optional<json> search_helper(editor &ed) {
    std::string key_as_str = read_string_from(ed.in());

    auto key_as_pmr = string_to_trivial_json(key_as_str);
    if (!key_as_pmr) {
        ed.out() << "Invalid trivial JSON type was entered.\n";
        return {};
    }

    const json_parser::json result = ed.draft().extract_mapped_if(
        [target = key_as_pmr->clone()](
            const json::value &key, const json::value &) {
            return key == *target;
        });

    return result;
}

static mystd::optional<json_parser::json::path> read_path(editor &ed, std::string prompt = "path") {
    json_parser::json::path path;
    std::string path_begin;
    ed.out() << "Enter " << prompt << ": ";
    ed.in() >> path_begin;
    if (path_begin != "["){
        ed.out() << "Invalid path - it should start with '[' and end with ']'.\n";
        return {};
    }

    for (std::string content; ; content.clear()) {
        content = read_string_from(ed.in());
        if (content == "]")
            break;
        auto path_component = string_to_trivial_json(content);
        if (!path_component)
            ed.out() << "Invalid path component used '" + content + "'.\n";
        path.push_back(std::move(path_component));
    }

    return path;
}

enum class with_object {
    KeyOnly,
    KeyAndMapped
};

template <with_object Pref = with_object::KeyAndMapped, typename Func>
static void with_new_object_element(editor &ed, Func action) {
    ed.out() << "Enter key: ";
    std::string key_str = read_string_from(ed.in());
    json_parser::json::pmrvalue key = string_to_trivial_json(key_str);
    if (!key) {
        ed.out() << "Invalid new node - it has to be a valid JSON trivial type.\n";
        return;
    }
    auto *key_as_str = dynamic_cast<json_parser::json::string *>(key.get());
    if (!key_as_str) {
        ed.out() << "Error: Expecting json::string as key.\n";
        return;
    }

    using enum with_object;
    if constexpr (Pref == KeyOnly) {
        action(*key_as_str);
        return;
    }

    ed.out() << "Enter mapped:\n";
    std::string mapped_str = read_snippet_from(ed.in());
    json_parser::json::pmrvalue mapped;
    try {
        json_parser::json parsed = json_parser::str_parser{json_parser::str_input_reader{mapped_str}}();
        mapped = parsed.take();
    } catch (const json_parser::parser_exception &pe) {
        ed.out() << "Error: " + std::string(pe.what()) << '\n';
        return;
    }

    if constexpr (Pref == KeyAndMapped) {
        action(*key_as_str, mystd::move(mapped));
        return;
    }

    // Safety: Key and KeyAndMapped are the only available options.
    mystd::unreachable();
}

template <typename Func>
static void with_new_array_element(editor &ed, Func action) {
    ed.out() << "Enter key: ";
    json_parser::json::pmrvalue key;
    try {
        std::string key_as_str = read_snippet_from(ed.in());
        json_parser::json parsed = json_parser::str_parser{json_parser::str_input_reader{key_as_str}}();
        key = parsed.take();
    } catch (const json_parser::parser_exception &pe) {
        ed.out() << "Error: " + std::string(pe.what()) << '\n';
    }

    action(mystd::move(key));
}

///
/// Commands implementation
///

bool open_cmd(editor &ed) {
    if (ed.active()) {
        ed.out() << "File '" + ed.draft_origin() + "' is already opened. Close it first before opening another file.\n";
        return false;
    }

    std::string filename;
    ed.in() >> filename;

    try {
        json_parser::ifs_input_reader ifs_input{filename};
        ed.draft() = json_parser::ifs_parser{std::move(ifs_input)}();
        ed.set_draft_origin(filename);
    } catch (const std::exception &e) {
        ed.out() << "Error: " + std::string{e.what()} << '\n';
        return false;
    }

    ed.out() << "File '" + filename + "' parsed successfully\n";
    return false;
}

bool close_cmd(editor &ed) {
    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    ed.draft() = json_parser::json();
    ed.set_draft_origin("");
    ed.out() << "Draft closed successfully.\n";
    return false;
}

bool save_cmd(editor &ed) {


    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    std::ofstream out_ifs{ed.draft_origin(), std::ios::out | std::ios::trunc};
    ed.draft().dump(out_ifs);
    return false;
}

bool saveas_cmd(editor &ed) {
    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    std::string out_filename;
    ed.in() >> out_filename;
    std::ofstream out_ifs{out_filename, std::ios::out | std::ios::trunc};
    ed.draft().dump(out_ifs);
    return false;
}

bool validate_cmd(editor &ed) {
    return open_cmd(ed);
}

bool print_cmd(editor &ed) {
    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    ed.draft().dump(ed.out());
    ed.out() << '\n';
    return false;
}

bool search_cmd(editor &ed) {
    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    if (auto result = search_helper(ed); result){
        result->dump(ed.out());
        ed.out() << '\n';
    }
    return false;
}

bool contains_cmd(editor &ed) {
    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    if (auto result = search_helper(ed); result)
        ed.out() << (result->empty() ? "Key does not exist." : "Key exists.") << '\n';
    return false;
}

bool set_cmd(editor &ed) {
    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    auto maybe_path = read_path(ed);
    if (!maybe_path)
        return false;
    auto &path = *maybe_path;

    json_parser::json::pmrvalue *node{nullptr};
    try {
        node = &ed.draft().follow(path);
    } catch (const json_parser::json_exception &je) {
        ed.out() << "Invalid path used - it does not exist.\n";
        return false;
    }

    // Safety: follow either throws an error or returns a valid ptr to a JSON node.
    assert(node != nullptr);

    ed.out() << "Looking at:\n";
    (*node)->serialize(ed.out(), /* depth */ 0);
    ed.out() << '\n';

    ed.out() << "Enter the new node: ";
    std::string new_node_str = read_string_from(ed.in());
    json_parser::json::pmrvalue new_node = string_to_trivial_json(new_node_str);
    if (!new_node) {
        ed.out() << "Invalid new node - it has to be a valid JSON trivial type.\n";
        return false;
    }

    node->swap(new_node);
    return false;
}

static void create_object_element(editor &ed, json::object &node) {
    ed.out() << "Enter key: ";
    std::string key_str = read_string_from(ed.in());
    json_parser::json::pmrvalue key = string_to_trivial_json(key_str);
    if (!key) {
        ed.out() << "Invalid new node - it has to be a valid JSON trivial type.\n";
        return;
    }
    auto *key_as_str = dynamic_cast<json_parser::json::string *>(key.get());
    if (!key_as_str) {
        ed.out() << "Error: Expecting json::string as key.\n";
        return;
    }

    ed.out() << "Enter mapped:\n";
    std::string mapped_str = read_snippet_from(ed.in());
    json_parser::json::pmrvalue mapped;
    try {
        json_parser::json parsed = json_parser::str_parser{json_parser::str_input_reader{mapped_str}}();
        mapped = parsed.take();
    } catch (const json_parser::parser_exception &pe) {
        ed.out() << "Error: " + std::string(pe.what()) << '\n';
        return;
    }

    if (node.contains(*key_as_str)) {
        ed.out() << "Error: This key already exists.\n";
        return;
    }

    node.append(*key_as_str, mystd::move(mapped));
}

static void create_array_element(editor &ed, json::array &node) {
    ed.out() << "Enter key: ";
    json_parser::json::pmrvalue key;
    try {
        std::string key_as_str = read_snippet_from(ed.in());
        json_parser::json parsed = json_parser::str_parser{json_parser::str_input_reader{key_as_str}}();
        key = parsed.take();
    } catch (const json_parser::parser_exception &pe) {
        ed.out() << "Error: " + std::string(pe.what()) << '\n';
    }

    if (node.contains(*key)) {
        ed.out() << "Error: This key already exists.\n";
        return;
    }

    node.append(mystd::move(key));
}

bool create_cmd(editor &ed) {
    json_parser::json::pmrvalue *pmrnode = follow_path_subcmd(ed);
    json_parser::json::value *node = pmrnode->get();
    if (!node)
    if (!ed.active()) {
        ed.out() << "No file is opened.\n";
        return false;
    }

    auto maybe_path = read_path(ed);
    if (!maybe_path)
        return false;
    auto &path = *maybe_path;

    json_parser::json::value *node{nullptr};
    try {
        node = ed.draft().follow(path);
    } catch (const json_parser::json_exception &je) {
        ed.out() << "Invalid path used - it does not exist.\n";
        return false;
    }

    // Safety: follow either throws an error or returns a valid ptr to a JSON node.
    assert(node != nullptr);

    ed.out() << "Looking at:\n";
    node->serialize(ed.out(), /* depth */ 0);
    ed.out() << '\n';

    if (node->trivial()) {
        ed.out() << "Cannot add elements to a trivial JSON type.\n";
        return false;
    }

    using enum with_object;
    if (auto *node_as_object = dynamic_cast<json::object *>(node); node_as_object)
        with_new_object_element<KeyAndMapped>(ed, [&](auto &&key, auto &&mapped) {
            if (node_as_object->contains(key)) {
                ed.out() << "Error: This key already exists.\n";
                return;
            }

            node_as_object->append(key, mystd::forward<decltype(mapped)>(mapped));
        });
    else if (auto *node_as_array = dynamic_cast<json::array *>(node); node_as_array)
        with_new_array_element(ed, [&](auto &&key){
            if (node_as_array->contains(*key)) {
                ed.out() << "Error: This key already exists.\n";
                return;
            }

            node_as_array->append(mystd::forward<decltype(key)>(key));
        });

    return false;
}

bool delete_cmd(editor &) {
    return false;
}

bool move_cmd(editor &) {
    return false;
}

bool exit_cmd(editor &ed) {
    ed.out() << "Bye :)";
	return true;
}

bool help_cmd(editor &ed) {
    ed.out() << "The json-editor manual:\n\n";
    ed.out() << "\t- open file.json\n";
    ed.out() << "\t- close\n";
    ed.out() << "\t- save\n";
    ed.out() << "\t- saveas newfile.json\n";
    ed.out() << "\t validate file.json\n";
    ed.out() << "\t- print out.json\n";
    ed.out() << "\t- search key\n";
    ed.out() << "\t- contains key\n";
    ed.out() << "\t- set key val\n";
    ed.out() << "\t- create path node\n";
    ed.out() << "\t- delete path\n";
    ed.out() << "\t- move dest_path src_path\n";
    ed.out() << "\t- exit\n";
    ed.out() << "\t- help\n";
    ed.out() << "\n\n";
    return false;
}

} // namespace json_editor::commands
