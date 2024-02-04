#!/bin/sh
set -ex

# Readlink compatibility function
readlinkf() {
    local target
    target=$1

    cd "$(dirname "$target")" || exit
    target=$(basename "$target")

    while [[ -L $target ]]; do
        target=$(readlink "$target")
        cd "$(dirname "$target")" || exit
        target=$(basename "$target")
    done

    echo "$(pwd -P)/$target"
}

root_dir=$(readlinkf "$(dirname "$(readlinkf "$0")")/..")

image_name=mcgraphxp

cd "${root_dir}"
export DOCKER_BUILDKIT=0

uid=$(id -u)
gid=$(id -g)

docker build \
  --file "${root_dir}/.docker/env.dockerfile" \
  --build-arg=UID="${uid}" \
  --build-arg=GID="${gid}" \
  --tag "${image_name}" \
  .

guest_ws_dir=/home/user/workspace

docker run \
  --rm \
  -ti \
  --volume "${root_dir}:${guest_ws_dir}" \
  --workdir ${guest_ws_dir} \
  --env OUT=out \
  --user "${uid}:${gid}" \
  -p 8888:8888 \
   "${image_name}" \
   $@
