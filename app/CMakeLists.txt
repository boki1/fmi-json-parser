add_executable(json-editor
		src/main.cpp
		src/editor.cpp)
target_compile_options(json-editor PUBLIC
	-Wall -Wextra -Werror -std=c++20)
target_include_directories(json-editor PUBLIC
	include/
	../mystd/include/
	../lib/include/)
target_link_libraries(json-editor PUBLIC
	json-parser
	mystd)
