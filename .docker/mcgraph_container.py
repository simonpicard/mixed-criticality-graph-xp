#!/usr/bin/env python3

from pathlib import Path

from pythainer.builders import UbuntuDockerBuilder
from pythainer.examples.builders import get_user_builder
from pythainer.examples.runners import personal_runner
from pythainer.runners import ConcreteDockerRunner


def get_mcgraph_builder(image_name: str) -> UbuntuDockerBuilder:
    # base_ubuntu_image = "ubuntu:22.04"
    base_ubuntu_image = "ubuntu:23.10"

    user_name = "user"
    lib_dir = "/home/${USER_NAME}/workspace/libraries"

    docker_builder = get_user_builder(
        image_name=image_name,
        base_ubuntu_image=base_ubuntu_image,
        user_name=user_name,
        lib_dir=lib_dir,
        packages=[
            "python3.11",
            "python3.11-dev",
            "python3.11-venv",
        ],
    )
    docker_builder.space()

    docker_builder.env(name="PYTHONDONTWRITEBYTECODE", value="yes")

    return docker_builder


def buildrun():
    image_name = "mcgraphxp"
    dockerfile = Path("~/git/ulb/xp/mixed-criticality-graph-xp/.docker/env.dockerfile").expanduser()

    docker_builder = get_mcgraph_builder(image_name=image_name)

    docker_builder.build(dockerfile_savepath=dockerfile)

    docker_runner = ConcreteDockerRunner(
        image=image_name,
        environment_variables={},
        volumes={},
        devices=[],
        network="host",
    )
    docker_runner |= personal_runner()

    cmd = docker_runner.get_command()
    print(" ".join(cmd))

    docker_runner.generate_script()


def main():
    buildrun()


if __name__ == "__main__":
    main()
