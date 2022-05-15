# This Dockerfile demonstrates the dependencies required and how to build FreeCG.
# It can also be used to actually build and run the game.
# To run, place the level files and GRAVITY.GFX from the original game (version 2.0E)
# in data/, then build the docker image:
# ```
# $ docker build . --tag freecg
# ```
# and then run:
# ```
# $ xhost +
# $ docker run --rm -ti -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY freecg data/LEVEL01.CGL
# ```
FROM ubuntu:jammy

ARG _APT_BEGIN_="apt-get update"
ARG _APT_END_="rm -fr /var/lib/apt/lists/*"

ENV DEBIAN_FRONTEND=noninteractive

RUN $_APT_BEGIN_ \
 && apt-get -y install \
 	gcc \
        make \
        libsdl1.2-dev \
	libsdl-image1.2-dev \
 && $_APT_END_

COPY ./ /cg
RUN cd /cg && make

WORKDIR /cg
ENTRYPOINT ["/cg/cgl_view"]
