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
image_rootless_name=rlmcgraphxp
volume_name=mcgraphxp_cache

cd "${root_dir}"
export DOCKER_BUILDKIT=0
if [ -z "${DOCKER_ROOTLESS}" ]
then
  DOCKER_ROOTLESS=0
else
  DOCKER_ROOTLESS=${DOCKER_ROOTLESS}
fi

uid=$(id -u)
gid=$(id -g)

if [ ! 0 = ${DOCKER_ROOTLESS} ]
then
  contuid=1100
  contgid=1100

  #shiftuid=$(cat /etc/subuid | grep $USER | cut -f 2 -d ':')
  #shiftgid=$(cat /etc/subgid | grep $USER | cut -f 2 -d ':')

  #hostuid=$(($shiftuid + $contuid))
  #hostgid=$(($shiftgid + $contgid))

  uid=$contuid
  gid=$contgid
fi

docker volume ls | grep -i "${volume_name}" || docker volume create "${volume_name}"

docker build \
  --file "${root_dir}/.docker/env.dockerfile" \
  --build-arg=UID="${uid}" \
  --build-arg=GID="${gid}" \
  --build-arg=IMAGE_VENV="${IMAGE_VENV}" \
  --tag "${image_name}" \
  --platform linux/amd64 \
  .

if [ "x0x" = "x${DOCKER_ROOTLESS}x" ]
then
  run_image=${image_name}
else
  docker build \
    --file "${root_dir}/.docker/rootless.dockerfile" \
    --build-arg=BASE_IMAGE="${image_name}" \
    --tag "${image_rootless_name}" \
    .

  run_image=${image_rootless_name}
fi

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
   "${run_image}" \
   $@
