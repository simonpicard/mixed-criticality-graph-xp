#!/bin/bash
set -e

# Define script directory, root directory, and venv directory
script_dir=$(dirname "$0")
root_dir=$(cd "$script_dir" && pwd)
venv_dir=$1

# Check if the venv directory is provided as an argument
if [ -z "$venv_dir" ]; then
  echo "Please provide the target venv directory on the command line." >&2
  exit 1
fi

PYTHON=python3.11
VENV="$venv_dir"
PIP="$VENV/bin/pip"

# Create a virtual environment, upgrade pip, and install requirements
$PYTHON -m venv "$VENV"
$PIP install --upgrade pip
$PIP install -r "${root_dir}/requirements.txt"
