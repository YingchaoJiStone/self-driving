function run {
    mkdir build/$1
    docker network create network-$1

    # RUN DEPENDENCIES
    cd ./recs
    docker run --rm -id --ipc=host --network network-$1 --init --name=opendlv-vehicle-view-$2 -v $PWD:/opt/vehicle-view/recordings -v /var/run/docker.sock:/var/run/docker.sock chrberger/opendlv-vehicle-view:v0.0.64 /bin/bash -c "/usr/bin/cluon-replay --cid=253 /opt/vehicle-view/recordings/CID-140-recording-$1-selection.rec"
    sleep 1
    docker run --rm -id --ipc=host --network network-$1 --name=decode-$2 -v /tmp/$2:/tmp h264decoder:v0.0.5 --cid=253 --name=img
    sleep 0.5

    # RUN PROGRAM
    cd ../build
    docker run --rm -i --ipc=host --network network-$1 --name=toolchain-$2 -e DISPLAY=$DISPLAY -v /tmp/$2:/tmp -v ./$1:/out toolchain:1.0 --cid=253 --name=img --width=640 --height=480 >>log.txt

    # PLOT
    docker build -f ../scripts/Dockerfile -t plot-$2 ./$1
    docker create --name plot_container-$2 plot-$2
    docker cp plot_container-$2:/out/. $1
    docker rm plot_container-$2

    # CLEANUP
    docker network rm network-$1
}

# --------------------------- BUILD -------------------------- #

docker build https://github.com/chalmers-revere/opendlv-video-h264-decoder.git#v0.0.5 -f Dockerfile -t h264decoder:v0.0.5

cd ..

if ! docker image inspect toolchain:1.0 >/dev/null 2>&1; then
    docker build --build-arg=DO_TEST=false -f Dockerfile -t toolchain:1.0 .
else
    echo " === IMAGE ALREADY EXISTS. SKIPPING BUILD!!! === "
fi

# ---------------------------- RUN --------------------------- #

run 2020-03-18_144821 8081 &
run 2020-03-18_145043 8080 &
run 2020-03-18_145233 8888 &
run 2020-03-18_145641 9000 &

# START STOP-SCRIPT
cd scripts
./stopAtEnd.sh &

# --------------------------- OTHER -------------------------- #

cd ..
run 2020-03-18_150001 9090
sleep 2
