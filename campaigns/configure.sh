#!/bin/sh
set -e

if [ "$INSIDE_DOCKER" = "1" ]; then
  MACH="docker"
else
  MACH="host"
fi
VENV_NAME=".venv-$(uname -m)-${MACH}"

../deps/benchkit/scripts/install_venv.sh "${VENV_NAME}"
