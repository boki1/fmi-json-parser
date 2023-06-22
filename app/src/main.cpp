#include <iostream>

#include <json-parser/json.h>
#include <json-parser/parser.h>

#include <json-editor/editor.h>

using namespace json_editor;

int main() {
    editor cmdline { "Greetings!"};

    cmdline.add_cmd("help", commands::help_cmd);
	cmdline.add_cmd("open", commands::open_cmd);
    cmdline.add_cmd("close", commands::close_cmd);
    cmdline.add_cmd("save", commands::save_cmd);
    cmdline.add_cmd("saveas", commands::saveas_cmd);
    cmdline.add_cmd("validate", commands::validate_cmd);
    cmdline.add_cmd("print", commands::print_cmd);
    cmdline.add_cmd("search", commands::search_cmd);
    cmdline.add_cmd("contains", commands::contains_cmd);
    cmdline.add_cmd("set", commands::set_cmd);
    cmdline.add_cmd("create", commands::create_cmd);
    cmdline.add_cmd("delete", commands::delete_cmd);
    cmdline.add_cmd("move", commands::move_cmd);
    cmdline.add_cmd("exit", commands::exit_cmd);

    cmdline.loop();

    return 0;
}
