#!/bin/bash

# Simple script for building and running the toolchain docker container with the main executable
# Feel free to make any modifications :)

cd ..

if [[ "$*" == *"--no-test"* ]]; then
    docker build --build-arg=DO_TEST=false -f Dockerfile -t toolchain:1.0 .
else
    docker build -f Dockerfile -t toolchain:1.0 .
fi

# Run if successful

if [ $? -eq 0 ]; then
    docker run --rm -ti --net=host --ipc=host -e DISPLAY=$DISPLAY -v /tmp:/tmp -v .:/out toolchain:1.0 --cid=253 --name=img --width=640 --height=480
else
    echo "Build failed!!"
fi
