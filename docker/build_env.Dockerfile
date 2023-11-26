FROM ubuntu:20.04

ENV OWNER=binary_ho

RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    wget \
    gcc \
    g++; \
    mkdir -p /usr/lib/heat