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

bool open_cmd(editor &ed) {
    if (ed.active()) {
        ed.out() << "File '" + ed.draft_origin() + "' is already opened. Close it first before opening another file.\n";
        return false;
    }

    std::string filename;
    ed.in() >> filename;
    ed.draft() = json_parser::parser{filename}();
    ed.set_draft_origin(filename);
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

bool search_cmd(editor &) {
    return false;
}

bool contains_cmd(editor &) {
    return false;
}

bool set_cmd(editor &) {
    return false;
}

bool create_cmd(editor &) {
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
