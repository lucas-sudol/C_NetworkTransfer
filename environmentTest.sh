#!/bin/bash

# Usage: ./environmentTest.sh <fileName> <server-hostname>:<port-number> <bufferSize> <numTransfers>
fileName=$1
serverAddress=$2
bufferSize=$3
numTransfers=$4

logFile="transfer_results.log"
pingFile="ping_results.log"

# Determine file size in bytes
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    fileSize=$(stat --printf="%s" "$fileName")
elif [[ "$OSTYPE" == "darwin"* ]]; then
    fileSize=$(stat -f%z "$fileName")
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

# Extract the hostname/IP from the server address
host=$(echo "$serverAddress" | cut -d':' -f1)

# Step 1: Run ping and save results
echo "Pinging $host..." > $pingFile
ping -c 20 "$host" >> $pingFile

# Step 2: Log file transfer details
echo "File Size: $fileSize bytes" > $logFile
echo "Server: $serverAddress" >> $logFile
echo "Buffer Size: $bufferSize bytes" >> $logFile

totalTime=0
minTime=99999
maxTime=0

for ((i = 1; i <= numTransfers; i++)); do
    echo "Transfer $i..." >> $logFile

    # Measure transfer time using the `time` command
    { time ./sendFile "$fileName" "$serverAddress" "$bufferSize"; } 2> temp_time.log
    elapsed=$(grep real temp_time.log | awk '{print $2}')

    # Parse the elapsed time
    if [[ "$elapsed" =~ ([0-9]+)m([0-9]+\.[0-9]+)s ]]; then
        minutes=${BASH_REMATCH[1]}
        seconds=${BASH_REMATCH[2]}
        elapsedSeconds=$(echo "$minutes * 60 + $seconds" | bc)
    elif [[ "$elapsed" =~ ([0-9]+\.[0-9]+)s ]]; then
        elapsedSeconds=${BASH_REMATCH[1]}
    else
        echo "Error: Unexpected time format: $elapsed" >> $logFile
        elapsedSeconds=0
    fi

    if (( $(echo "$elapsedSeconds > 0" | bc -l) )); then
        echo "Transfer $i Time: $elapsedSeconds seconds" >> $logFile
        totalTime=$(echo "$totalTime + $elapsedSeconds" | bc)
        if (( $(echo "$elapsedSeconds < $minTime" | bc -l) )); then
            minTime=$elapsedSeconds
        fi
        if (( $(echo "$elapsedSeconds > $maxTime" | bc -l) )); then
            maxTime=$elapsedSeconds
        fi
    else
        echo "Warning: Transfer $i reported zero time; skipping metrics update." >> $logFile
    fi
done

avgTime=$(echo "$totalTime / $numTransfers" | bc -l)
avgRate=$(echo "$fileSize / $avgTime" | bc -l)
minRate=$(echo "$fileSize / $maxTime" | bc -l)
maxRate=$(echo "$fileSize / $minTime" | bc -l)

echo "Summary:" >> $logFile
echo "Min Transfer Time: $minTime seconds" >> $logFile
echo "Max Transfer Time: $maxTime seconds" >> $logFile
echo "Avg Transfer Time: $avgTime seconds" >> $logFile
echo "Min Transfer Rate: $minRate bytes/second" >> $logFile
echo "Max Transfer Rate: $maxRate bytes/second" >> $logFile
echo "Avg Transfer Rate: $avgRate bytes/second" >> $logFile

# Cleanup temporary time log
rm temp_time.log
