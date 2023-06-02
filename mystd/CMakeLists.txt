add_library(mystd INTERFACE)
target_include_directories(mystd INTERFACE include/)

# Pass this option in order to enable all of the custom implementaion provided
# in "mystd" instead of only aliasing standard types.
if(FMI_JSON_PARSER_BUILD_WITH_MYSTD)
    target_compile_definitions(mystd INTERFACE FMI_JSON_PARSER_BUILD_WITH_MYSTD)
endif()

target_compile_options(mystd INTERFACE
	-Wall -Wextra -Werror -std=c++20)

