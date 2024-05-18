#!/bin/sh
set -ex

if [ "$INSIDE_DOCKER" = "1" ]; then
  MACH="docker"
else
  MACH="host"
fi
VENV_NAME=".venv-$(uname -m)-${MACH}"

./${VENV_NAME}/bin/python3 campaign_test.py
