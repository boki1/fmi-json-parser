add_library(json-parser
		src/tokenizer.cpp
		src/parser.cpp)
target_compile_options(json-parser PUBLIC
	-Wall -Wextra -Werror -std=c++20)
target_include_directories(json-parser PUBLIC
	include/)
target_link_libraries(json-parser PUBLIC
		mystd)
target_include_directories(json-parser PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/../mystd/include)
