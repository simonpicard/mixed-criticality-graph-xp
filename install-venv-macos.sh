#!/bin/sh
set -e

script_dir=$(greadlink -e "$(dirname "$0")")
root_dir=$(greadlink -e "${script_dir}")
venv_dir=$1

if [ -z "${venv_dir}" ]
then
  echo "Please provide the target venv directory on command line." >&2
  exit 1
fi

PYTHON=python3.11
VENV=${venv_dir}
PIP=${VENV}/bin/pip

${PYTHON} -m venv "${VENV}"
${PIP} install --upgrade pip
${PIP} install --requirement "${root_dir}/requirements.txt"
