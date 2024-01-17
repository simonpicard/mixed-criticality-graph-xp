#!/bin/sh
set -ex

root_dir=$(readlink -e "$(dirname "$(readlink -e "$0")")/..")

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
  --network=host \
  --user "${uid}:${gid}" \
   "${image_name}" \
   $@
