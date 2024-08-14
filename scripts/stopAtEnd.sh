#!/bin/sh

# Inspired by Google Gemini

sleep 10
filename="../build/log.txt"

while true; do
    last_modified=$(stat -c %y "$filename")

    sleep 5

    if [ "$last_modified" = "$(stat -c %y "$filename")" ]; then
        docker stop $(docker ps -q)
        exit 0
    fi
done
