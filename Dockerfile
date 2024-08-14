
# ======================== BUILD STAGE ======================= #

FROM ubuntu:18.04 as Builder

# ------------------- ENVIRONMENT VARIABLES ------------------ #

ENV DEBIAN_FRONTEND=noninteractive
ENV leakcheck="valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --error-exitcode=863 \ 
    --show-possibly-lost=no \
    --errors-for-leak-kinds=definite"

# ------------------------- SETUP OS ------------------------- #

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y

RUN mkdir /out

# ------------------- INSTALL DEPENDENCIES ------------------- #

RUN apt-get install -y \
    cmake \
    libopencv-dev \
    build-essential

ARG DO_TEST=true 

RUN if [ "${DO_TEST}" = "true" ]; then \
    apt-get install -y \
    lcov \
    valgrind; \
    fi

# -------------------- IMPORT SOURCE CODE -------------------- #

ADD . /opt/sources
WORKDIR /opt/sources

# -------------------- SETUP BUILD FOLDER ------------------- #

RUN [ -d build ] && rm -rf build || true
RUN mkdir build
WORKDIR /opt/sources/build

# --------------------- BUILD AND TEST ----------------------- #

RUN cmake -D CMAKE_BUILD_TYPE=Release .. && \
    make

RUN if [ "${DO_TEST}" = "true" ]; then \
    make test && \ 
    make coverage; \
    fi

# --------------------------- MEMORY LEAK TEST -------------------------- #

RUN if [ "${DO_TEST}" = "true" ]; then \
    ${leakcheck} ./main || true && \
    if [ $? -eq 863 ]; then \
    echo "Memory leaks detected"; \
    exit 1; \
    else \
    echo "No memory leaks detected"; \
    fi \
    fi

# --------------------------- OTHER -------------------------- #

# Strips the binary
RUN strip -s ./main

# ======================= BUNDLE STAGE ======================= #

FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y

RUN apt-get install -y \
    libopencv-dev

COPY --from=builder /opt/sources/build /build

# Sets the entrypoint to the binary
ENTRYPOINT ["/build/main"]
