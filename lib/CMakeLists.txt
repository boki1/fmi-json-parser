add_library(json-parser
		src/lexer.cpp
		src/parser.cpp)
target_compile_options(json-parser PUBLIC
	-Wall -Wextra -Werror -std=gnu++2b)
target_include_directories(json-parser PUBLIC
	include)

target_link_libraries(json-parser PUBLIC
		mystd)
target_include_directories(json-parser PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/../mystd/include)
