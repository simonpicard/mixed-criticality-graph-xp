#!/bin/sh
set -ex

root_dir=$(readlink -e "$(dirname $0)")
cpp_dir=$(readlink -e "${root_dir}/mcsexplorer")

find "${cpp_dir}" -iname "*.cpp" -o -iname "*.h" | xargs clang-format -i
