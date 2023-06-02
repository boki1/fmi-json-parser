#!/bin/bash

# This is the project build script.
# Pass --help to review how to use the script.

output_dir=build
should_configure="yes"
should_ask="no"
should_clean="no"

function help() {
	echo "Usage: $0 [--output=] [--skip-ccmake]"
}

function build_procedure() {
	[ "${should_configure}" == "no" ] || \
		ccmake -S. -B${output_dir} -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug
	[ -f ${output_dir}/compile_commands.json ] && \
		ln -sf ${output_dir}/compile_commands.json compile_commands.json
	ninja -C${output_dir}
}

function clean() {
	question=$([ ${should_ask} == "yes" ] && echo "-i" || echo "")
	rm -rf ${question} ${output_dir}
}

function parse_arguments() {
	while [[ $# -gt 0 ]]; do
		case "$1" in
		--help)
			help
			exit
			;;
		--output=*)
			output_dir="${1#*=}"
			shift
			;;
		--configure=*)
			should_configure="${1#*=}"
			shift
			;;
		--clean)
			should_clean="yes"
			shift
			[[ "$1" == "--ask" ]] && should_ask="yes"
			clean
			;;
		*)
			echo "$0: Invalid input."
			exit
			;;
		esac
	done
}

function main() {
	parse_arguments $@
	build_procedure
}

main $@
