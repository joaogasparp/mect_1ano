#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ./PollingStationDeployAndRun.sh <stationCapacity>"
    exit 1
fi

CAPACITY=$1

echo "Setting up the polling station node."

rm -rf test/ElectionDay
mkdir -p test/ElectionDay

cp dirPollingStation.zip test/ElectionDay
echo "Decompressing data sent to the polling station node."
unzip -o test/ElectionDay/dirPollingStation.zip -d test/ElectionDay

echo "Executing program at the polling station node."
cd test/ElectionDay/dirPollingStation
java serverSide.main.ServerPollingStationMain $CAPACITY
