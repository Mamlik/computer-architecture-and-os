FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential gdb make \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /work
