#!/bin/sh
# Adjust the ownership of the mounted directory to match the host user
# which is root in rootless docker
user_name=$(whoami)
sudo chown -R root:root \
    "/home/${user_name}/.cache/pip" \
    "/home/${user_name}/workspace" \


# Execute the original command
exec "$@"
