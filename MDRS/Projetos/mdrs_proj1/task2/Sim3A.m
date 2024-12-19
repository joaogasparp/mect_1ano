function [PLdata, PLVoIP, APDdata, APDVoIP, MPDdata, MPDVoIP, TT] = Sim3A(lambda, C, f, P, n, b)
% INPUT PARAMETERS:
%  lambda - packet rate (packets/sec)
%  C      - link bandwidth (Mbps)
%  f      - queue size (Bytes)
%  P      - number of packets (stopping criterium)
%  n      - number of VoIP flows
%  b      - bit error rate
% OUTPUT PARAMETERS:
%  PLdata   - packet loss of data packets (%)
%  PLVoIP   - packet loss of VoIP packets (%)
%  APDdata  - average packet delay of data packets (milliseconds)
%  APDVoIP  - average packet delay of VoIP packets (milliseconds)
%  MPDdata  - maximum packet delay of data packets (milliseconds)
%  MPDVoIP  - maximum packet delay of VoIP packets (milliseconds)
%  TT       - transmitted throughput (data + VoIP) (Mbps)

% Events:
ARRIVAL_DATA = 0;       % Arrival of a data packet            
ARRIVAL_VOIP = 1;       % Arrival of a VoIP packet
DEPARTURE = 2;          % Departure of a packet

% State variables:
STATE = 0;              % 0 - connection is free; 1 - connection is occupied
QUEUEOCCUPATION = 0;    % Occupation of the queue (in Bytes)
QUEUE = [];             % Size and arriving time instant of each packet in the queue

% Statistical Counters:
TOTALPACKETS_DATA = 0;  % No. of data packets arrived to the system
LOSTPACKETS_DATA = 0;   % No. of data packets dropped due to buffer overflow
TRANSPACKETS_DATA = 0;  % No. of transmitted data packets
TRANSBYTES_DATA = 0;    % Sum of the Bytes of transmitted data packets
DELAYS_DATA = 0;        % Sum of the delays of transmitted data packets
MAXDELAY_DATA = 0;      % Maximum delay among all transmitted data packets

TOTALPACKETS_VOIP = 0;  % No. of VoIP packets arrived to the system
LOSTPACKETS_VOIP = 0;   % No. of VoIP packets dropped due to buffer overflow
TRANSPACKETS_VOIP = 0;  % No. of transmitted VoIP packets
TRANSBYTES_VOIP = 0;    % Sum of the Bytes of transmitted VoIP packets
DELAYS_VOIP = 0;        % Sum of the delays of transmitted VoIP packets
MAXDELAY_VOIP = 0;      % Maximum delay among all transmitted VoIP packets

% Initializing the simulation clock:
Clock = 0;

% Initializing the List of Events with the first ARRIVAL_DATA and ARRIVAL_VOIP:
tmp = Clock + exprnd(1/lambda);
EventList = [ARRIVAL_DATA, tmp, GeneratePacketSize(), tmp, ARRIVAL_DATA];

for i = 1:n
    tmp = Clock + (16 + (24-16)*rand())/1000;
    EventList = [EventList; ARRIVAL_VOIP, tmp, randi([110 130]), tmp, ARRIVAL_VOIP];
end

% Simulation loop:
while TRANSPACKETS_DATA + TRANSPACKETS_VOIP < P  % Stopping criterium
    EventList = sortrows(EventList, 2);  % Order EventList by time
    Event = EventList(1, 1);             % Get first event 
    Clock = EventList(1, 2);             %    and all
    PacketSize = EventList(1, 3);        %    associated
    ArrInstant = EventList(1, 4);        %    parameters.
    PacketType = EventList(1, 5);        %    packet type
    EventList(1, :) = [];                % Eliminate first event
    switch Event
        case ARRIVAL_DATA  % If first event is an ARRIVAL_DATA
            TOTALPACKETS_DATA = TOTALPACKETS_DATA + 1;
            tmp = Clock + exprnd(1/lambda);
            EventList = [EventList; ARRIVAL_DATA, tmp, GeneratePacketSize(), tmp, ARRIVAL_DATA];
            if STATE == 0
                STATE = 1;
                EventList = [EventList; DEPARTURE, Clock + 8*PacketSize/(C*10^6), PacketSize, Clock, ARRIVAL_DATA];
            else
                if QUEUEOCCUPATION + PacketSize <= f
                    QUEUE = [QUEUE; PacketSize, Clock, ARRIVAL_DATA];
                    QUEUEOCCUPATION = QUEUEOCCUPATION + PacketSize;
                else
                    LOSTPACKETS_DATA = LOSTPACKETS_DATA + 1;
                end
            end
        case ARRIVAL_VOIP  % If first event is an ARRIVAL_VOIP
            TOTALPACKETS_VOIP = TOTALPACKETS_VOIP + 1;
            tmp = Clock + (16 + (24-16)*rand())/1000;
            EventList = [EventList; ARRIVAL_VOIP, tmp, randi([110 130]), tmp, ARRIVAL_VOIP];
            if STATE == 0
                STATE = 1;
                EventList = [EventList; DEPARTURE, Clock + 8*PacketSize/(C*10^6), PacketSize, Clock, ARRIVAL_VOIP];
            else
                if QUEUEOCCUPATION + PacketSize <= f
                    QUEUE = [QUEUE; PacketSize, Clock, ARRIVAL_VOIP];
                    QUEUEOCCUPATION = QUEUEOCCUPATION + PacketSize;
                else
                    LOSTPACKETS_VOIP = LOSTPACKETS_VOIP + 1;
                end
            end
        case DEPARTURE  % If first event is a DEPARTURE
            if isPacketCorrupted(PacketSize, b)
                if PacketType == ARRIVAL_DATA
                    LOSTPACKETS_DATA = LOSTPACKETS_DATA + 1;
                else
                    LOSTPACKETS_VOIP = LOSTPACKETS_VOIP + 1;
                end
            else
                if PacketType == ARRIVAL_DATA
                    TRANSBYTES_DATA = TRANSBYTES_DATA + PacketSize;
                    DELAYS_DATA = DELAYS_DATA + (Clock - ArrInstant);
                    if Clock - ArrInstant > MAXDELAY_DATA
                        MAXDELAY_DATA = Clock - ArrInstant;
                    end
                    TRANSPACKETS_DATA = TRANSPACKETS_DATA + 1;
                else
                    TRANSBYTES_VOIP = TRANSBYTES_VOIP + PacketSize;
                    DELAYS_VOIP = DELAYS_VOIP + (Clock - ArrInstant);
                    if Clock - ArrInstant > MAXDELAY_VOIP
                        MAXDELAY_VOIP = Clock - ArrInstant;
                    end
                    TRANSPACKETS_VOIP = TRANSPACKETS_VOIP + 1;
                end
            end
            if QUEUEOCCUPATION > 0
                EventList = [EventList; DEPARTURE, Clock + 8*QUEUE(1,1)/(C*10^6), QUEUE(1,1), QUEUE(1,2), QUEUE(1,3)];
                QUEUEOCCUPATION = QUEUEOCCUPATION - QUEUE(1,1);
                QUEUE(1,:) = [];
            else
                STATE = 0;
            end
    end
end

% Performance parameters determination:
PLdata = 100 * LOSTPACKETS_DATA / TOTALPACKETS_DATA;  % in percentage
PLVoIP = 100 * LOSTPACKETS_VOIP / TOTALPACKETS_VOIP;  % in percentage
APDdata = 1000 * DELAYS_DATA / TRANSPACKETS_DATA;     % in milliseconds
APDVoIP = 1000 * DELAYS_VOIP / TRANSPACKETS_VOIP;     % in milliseconds
MPDdata = 1000 * MAXDELAY_DATA;                       % in milliseconds
MPDVoIP = 1000 * MAXDELAY_VOIP;                       % in milliseconds
TT = 1e-6 * (TRANSBYTES_DATA + TRANSBYTES_VOIP) * 8 / Clock;  % in Mbps

end

function out = GeneratePacketSize()
    aux = rand();
    aux2 = [65:109 111:1517];
    if aux <= 0.19
        out = 64;
    elseif aux <= 0.19 + 0.23
        out = 110;
    elseif aux <= 0.19 + 0.23 + 0.17
        out = 1518;
    else
        out = aux2(randi(length(aux2)));
    end
end

function isCorrupted = isPacketCorrupted(PacketSize, b)
    nBits = PacketSize * 8;
    pNoError = (1 - b)^nBits;
    isCorrupted = rand() > pNoError;
end