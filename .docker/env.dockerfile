FROM ubuntu:23.10

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        apt-utils \
    && rm -rf /var/lib/apt/lists/*

# General packages & tools
RUN apt-get update && apt-get install -y --no-install-recommends \
        apt-transport-https \
        build-essential \
        ca-certificates \
        curl \
        file \
        gdb \
        git \
        gnupg \
        less \
        libssl-dev \
        locales \
        locales-all \
        lsb-release \
        ninja-build \
        software-properties-common \
        sudo \
        telnet \
        tmux \
        tree \
        vim \
        wget \
    && rm -rf /var/lib/apt/lists/*

# Required packages
RUN apt-get update && apt-get install -y --no-install-recommends \
        python3.11 \
        python3.11-dev \
        python3.11-venv \
    && rm -rf /var/lib/apt/lists/*

# Set locales
ENV LC_ALL=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8

# Set root password
RUN echo 'root:root' | chpasswd

RUN yes | unminimize

# Create a non-root user
ARG USER_NAME=user
ARG UID
ARG GID
# Remove group with gid=${GID} if it already exists.
RUN grep :${GID}: /etc/group && \
    (grep :${GID}: /etc/group | \
     cut -d ':' -f 1 | \
     xargs groupdel) || \
    true
# Remove user with uid:gid=${UID}:${GID} if it already exists.
RUN grep :${UID}:${GID}: /etc/passwd && \
    (grep :${UID}:${GID}: /etc/passwd | \
     cut -d ':' -f 1 | \
     xargs userdel --remove) || \
    true
RUN groupadd -g ${GID} ${USER_NAME}
RUN adduser --disabled-password --uid $UID --gid $GID --gecos "" ${USER_NAME}
RUN adduser ${USER_NAME} sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN echo "${USER_NAME} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/10-docker
USER user

# Configure user environment
WORKDIR /home/${USER_NAME}
RUN touch ~/.sudo_as_admin_successful
RUN mkdir workspace
WORKDIR /home/${USER_NAME}/workspace

RUN mkdir -p /home/${USER_NAME}/workspace/libraries

# Build & install CMake from source
WORKDIR /home/${USER_NAME}/workspace/libraries
ARG cmake_version=3.27.9
RUN wget --quiet https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}.tar.gz
RUN tar -xf cmake-${cmake_version}.tar.gz
WORKDIR cmake-${cmake_version}
RUN ./bootstrap --parallel=$(nproc) && \
    make -j $(nproc) && \
    sudo make install && \
    (rm -rf /home/${USER_NAME}/workspace/libraries/cmake-${cmake_version} || true) && \
    (sudo chown -f --recursive ${USER_NAME}:${USER_NAME} /home/${USER_NAME}/workspace/libraries/cmake-${cmake_version} || true) && \
    rm -rf /home/${USER_NAME}/workspace/libraries/cmake-${cmake_version}

ENV PYTHONDONTWRITEBYTECODE=yes
