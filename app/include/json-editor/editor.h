#ifndef FMI_JSON_PARSER_APP_EDITOR_INCLUDED
#define FMI_JSON_PARSER_APP_EDITOR_INCLUDED

#include <string>
#include <iostream>

#include <json-parser/json.h>
#include <json-parser/parser.h>

#include <mystd/unordered_map.h>

namespace json_editor {

class editor {
    static const inline std::string prompt = "> ";

public:
    editor(const std::string &greeting, std::ostream &os = std::cout, std::istream &is = std::cin)
        : m_out{os}
        , m_in{is}
    {
        m_out << greeting << '\n';
    }

public:
    // Returns whether to exit the command entering process.
    using cmd_type = bool (*)(editor &);

    void add_cmd(const std::string &cmd_name, cmd_type func);

    void remove_cmd(const std::string &cmd_name);

    bool perform_cmd();

    void loop();

public:
  std::ostream &out() noexcept { return m_out; }
  std::istream &in() noexcept { return m_in; }

  using json_type = json_parser::json;

  [[nodiscard]] const json_type &draft() const & { return m_draft; }
  [[nodiscard]] json_type &draft() & { return m_draft; }

  void set_draft_origin(const std::string &o) { m_draft_origin = o; }

  [[nodiscard]] const std::string &draft_origin() const noexcept {
        return m_draft_origin;
  }

  [[nodiscard]] bool active() const noexcept { return !m_draft_origin.empty(); }

private:
    mystd::unordered_map<std::string, cmd_type> m_commands;
    std::ostream &m_out;
    std::istream &m_in;

    std::string m_draft_origin;
    json_type m_draft;
};

namespace commands {

// Command name: open
// Input: file name
// Side effect: the draft object contains the content of the
//              passed file if it is valid JSON
bool open_cmd(editor &);

// Command name: close
// Input: n/a
// Side effect: the draft object is emptied
bool close_cmd(editor &);

// Command name: save
// Input: n/a
// Side effect: the draft object's contents are serialized
//              to the original file that contained them
bool save_cmd(editor &);

// Command name: saveas
// Input: file name
// Side effect: the draft object's contents are serialized
//              to a file with the provided name
bool saveas_cmd(editor &);

// Command name: validate
// Input: file name
// Side effect: essentially the same as open
bool validate_cmd(editor &);

// Command name: print
// Input: file name
// Side effect: essentially the same as saveas
bool print_cmd(editor &);

// Command name: search
// Input: json::string key
// Side effect: returns a list of all occurances
bool search_cmd(editor &);

// Command name: contains
// Input: trival type
// Side effect: returns a boolean answer depending on whether
//              the provided key was found in the draft
bool contains_cmd(editor &);

// Command name: contains
// Input: dest_path, trivial type
// Side effect: sets the node at `dest_path` to the passed
//              trivial type
bool set_cmd(editor &);

// Command name: contains
// Input: key as a trival type
// Side effect: returns a boolean answer depending on whether
//              the provided key was found in the draft
bool create_cmd(editor &);

// Command name: contains
// Input: target_path
// Side effect: removes all nodes under `target_path`
bool delete_cmd(editor &);

// Command name: contains
// Input: dest_path, src_path
// Side effect: all nodes under `src_path should be moved under
//              `dest_path`
bool move_cmd(editor &);

// Command name: exit
// Input: n/a
// Side effect: exits the editor
bool exit_cmd(editor &);

// Command name: help
// Input n/a
// Side effect: print manual
bool help_cmd(editor &);

static_assert(std::is_same_v<decltype(&open_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&close_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&save_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&saveas_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&validate_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&print_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&search_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&contains_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&set_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&create_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&delete_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&move_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&exit_cmd), editor::cmd_type>);
static_assert(std::is_same_v<decltype(&help_cmd), editor::cmd_type>);

} // namespace commands

}

#endif // FMI_JSON_PARSER_APP_EDITOR_INCLUDED
