#!/bin/bash

# Use this script to build any singular library of the repository.
# Pass --help to review how to use the script.

output_dir=build
skip_ccmake="no"
ask="no"
should_clean="no"

function help() {
	echo "Usage: $0 [--output=] [--skip-ccmake]"
}

function build_procedure() {
	conan install . --output-folder=${output_dir} -if=${output_dir} --build=missing
	cmake -S. -B${output_dir} -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	ln -sf build/compile_commands.json compile_commands.json
	[ "${skip_ccmake}" == "no" ] && ccmake ${output_dir}
	echo "configuration: $?"
	ninja -C${output_dir}
}

function clean() {
	question=$([ ${ask} == "yes" ] && echo "-i" || echo "")
	rm -r ${question} ${output_dir}
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
			--skip-ccmake)
				skip_ccmake="yes"
				shift
				;;
			--clean)
				should_clean="yes"
				shift
				[[ "$1" == "--ask" ]] && ask="yes"
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
