FROM ubuntu:18.04

RUN apt update
RUN apt install -y build-essential git autoconf libtool pkg-config libssl-dev libjansson-dev check libjson-c-dev valgrind

RUN git clone https://github.com/benmcollins/libjwt.git
WORKDIR /libjwt
RUN autoreconf -i
RUN ./configure
RUN make all
RUN make check
RUN make install
ENV PKG_CONFIG_PATH "${PKG_CONFIG_PATH}:/libjwt/libjwt"
ENV LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:/usr/local/lib"

WORKDIR /
COPY . /githubapp-token-refresh
WORKDIR /githubapp-token-refresh

RUN make build