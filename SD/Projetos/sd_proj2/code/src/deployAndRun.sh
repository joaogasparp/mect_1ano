#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: ./deployAndRun.sh <stationCapacity>"
  echo "stationCapacity must be between 2 and 5"
  exit 1
fi

CAPACITY=$1

if ! [[ "$CAPACITY" =~ ^[2-5]$ ]]; then
  echo "Error: stationCapacity must be a number between 2 and 5"
  exit 1
fi

gnome-terminal -- bash -c "./PollingStationDeployAndRun.sh $CAPACITY; exec bash"
gnome-terminal -- bash -c "./ExitPollDeployAndRun.sh; exec bash"