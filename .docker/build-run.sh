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

IMAGE_VENV=${IMAGE_VENV}
[ -z ${IMAGE_VENV} ] && IMAGE_VENV=OFF

INTERACTIVE=${INTERACTIVE}
[ -z ${INTERACTIVE} ] && INTERACTIVE=1

INTERACTIVE_FLAGS=""
[ 1 -eq ${INTERACTIVE} ] && INTERACTIVE_FLAGS="-ti"

image_name=mcgraphxp
volume_name=mcgraphxp_cache

cd "${root_dir}"
export DOCKER_BUILDKIT=0

uid=$(id -u)
gid=$(id -g)

docker volume ls | grep -i "${volume_name}" || docker volume create "${volume_name}"

docker build \
  --file "${root_dir}/.docker/env.dockerfile" \
  --build-arg=UID="${uid}" \
  --build-arg=GID="${gid}" \
  --build-arg=IMAGE_VENV="${IMAGE_VENV}" \
  --tag "${image_name}" \
  .

guest_ws_dir=/home/user/workspace

docker run \
  --rm \
  ${INTERACTIVE_FLAGS} \
  --volume "${volume_name}":/home/user/.cache/pip \
  --volume "${root_dir}:${guest_ws_dir}" \
  --workdir ${guest_ws_dir} \
  --env OUT=out \
  --user "${uid}:${gid}" \
  -p 8888:8888 \
  --name "${image_name}" \
   "${image_name}" \
   $@
