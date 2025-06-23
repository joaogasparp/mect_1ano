#!/bin/bash

sudo iptables -A INPUT -p tcp --dport 3000 -j ACCEPT

# This script is used to run the dashboard application backend.
gnome-terminal -- bash -c "python3 server.py; exec bash"

# Wait 5 seconds.
sleep 5

# Open a new terminal for the npm front-end.
gnome-terminal -- bash -c "cd app; npm run start; exec bash"