#!/bin/sh
# Adjust the ownership of the mounted directory to match the container user
user_name=$(whoami)
sudo chown -R "${user_name}:${user_name}" \
    "/home/${user_name}/.cache/pip" \
    "/home/${user_name}/workspace" \


# Execute the original command
exec "$@"
