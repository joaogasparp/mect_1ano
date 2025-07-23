#!/bin/bash
# filepath: /cle_proj3/deploy.sh

# Define variables
REMOTE_USER="cle02"
REMOTE_SERVER="banana.ua.pt"
LOCAL_PATH="./cuda-canny"
PASSWORD="x7i5pfl" 

# Check if sshpass is installed
if ! command -v sshpass &> /dev/null; then
    echo "sshpass is not installed. Installing..."
    sudo apt-get update && sudo apt-get install -y sshpass
fi

# Copy the cuda-canny folder to the remote server
echo "Copying cuda-canny folder to remote server..."
sshpass -p "$PASSWORD" scp -r "$LOCAL_PATH" "$REMOTE_USER@$REMOTE_SERVER:~"

# Check if copy was successful
if [ $? -eq 0 ]; then
    echo "Copy successful!"
    
    # SSH into the remote server
    echo "Connecting to remote server..."
    sshpass -p "$PASSWORD" ssh "$REMOTE_USER@$REMOTE_SERVER"
else
    echo "Error: Failed to copy files to remote server."
    exit 1
fi
