#!/bin/bash

echo "Compiling source code."
javac */*.java */*/*.java

echo "Distributing intermediate code to the different execution environments."

# Polling Station
echo "  Polling Station"
rm -rf dirPollingStation
mkdir -p dirPollingStation/{serverSide/main,serverSide/entities,serverSide/sharedRegions,clientSide/entities,commInfra}
cp serverSide/main/ServerPollingStationMain.class dirPollingStation/serverSide/main
cp serverSide/entities/PollingStationService.class dirPollingStation/serverSide/entities
cp serverSide/sharedRegions/PollingStationInterface.class serverSide/sharedRegions/PollingStationSharedRegion.class dirPollingStation/serverSide/sharedRegions
cp clientSide/entities/VoterState.class dirPollingStation/clientSide/entities
cp commInfra/*.class dirPollingStation/commInfra

# Exit Poll
echo "  Exit Poll"
rm -rf dirExitPoll
mkdir -p dirExitPoll/{serverSide/main,serverSide/entities,serverSide/sharedRegions,clientSide/entities,commInfra}
cp serverSide/main/ServerExitPollMain.class dirExitPoll/serverSide/main
cp serverSide/entities/ExitPollService.class dirExitPoll/serverSide/entities
cp serverSide/sharedRegions/ExitPollInterface*.class serverSide/sharedRegions/ExitPollSharedRegion.class dirExitPoll/serverSide/sharedRegions
cp clientSide/entities/VoterState.class dirExitPoll/clientSide/entities
cp commInfra/*.class dirExitPoll/commInfra

echo "Compressing execution environments."
echo "  Polling Station"
rm -f dirPollingStation.zip
zip -r dirPollingStation.zip dirPollingStation

echo "  Exit Poll"
rm -f dirExitPoll.zip
zip -r dirExitPoll.zip dirExitPoll
