#!/usr/bin/env bash

GIT_DEPS_REPO=(tiny_math tiny_utils pybind11 imgui spdlog tiny_renderer tysoc googletest bullet3)
GIT_DEPS_USER=(wpumacay wpumacay RobotLocomotion wpumacay gabime wpumacay wpumacay google wpumacay)
GIT_DEPS_BRANCH=(master master drake docking v1.x master master master master)
GIT_DEPS_DEST=(ext/tiny_math ext/tiny_utils ext/pybind11 ext/imgui ext/spdlog ext/tiny_renderer core ext/googletest ext/bullet3)

for i in {0..8}
do
    USER=${GIT_DEPS_USER[$i]}
    REPO=${GIT_DEPS_REPO[$i]}
    BRANCH=${GIT_DEPS_BRANCH[$i]}
    URL=https://github.com/${USER}/${REPO}
    LOCATION=${GIT_DEPS_DEST[$i]}
    if [ -d "${LOCATION}" ]
    then
        echo "===> Deleting ${USER}/${REPO} ..."
        rm -rf ${LOCATION}
    else
        echo "===> Dependency ${USER}/${REPO} already deleted @ ${LOCATION}"
    fi
done

echo "Done!"