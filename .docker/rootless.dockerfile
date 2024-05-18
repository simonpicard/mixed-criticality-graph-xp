ARG BASE_IMAGE=rlmcgraphxp
FROM ${BASE_IMAGE}

# Entry point required to manage ownership in rootless docker
USER root
COPY .docker/entrypoint.sh /usr/local/bin/entrypoint.sh
COPY .docker/exitpoint.sh /usr/local/bin/exitpoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/bin/bash"]

USER ${USER_NAME}
WORKDIR /home/${USER_NAME}
