add_library(json-parser
		src/lexer.cpp
		src/parser.cpp)

target_include_directories(json-parser PUBLIC include)
