#!/bin/bash

# Usage: ./sendTest.sh <fileName> <IP-address>:<port-number> <number-of-clients> [bufSize]

fileName=$1
addressPort=$2
numClients=$3
bufSize=${4:-4096}

for ((i = 1; i <= numClients; i++)); do
    echo "Starting client $i..."
    ./sendFile "$fileName" "$addressPort" "$bufSize" &
    # sleep 1
done

# Wait for all background processes to finish
wait
echo "All clients finished."
