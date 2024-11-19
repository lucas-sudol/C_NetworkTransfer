# Project Title
A3 - Network Transfer Using C

## Description
Provides functionality for sending text files from a client to a server using TCP.

Supports host name resolution.

## Getting Started

### Dependencies
Linux 3090 Docker Containter

### Executing program
* Build the program with `make all`

* Start the server `./server port-number bufSize`

* Send a file using the client `./sendFile fileName IP-address:port-number bufSize`

* To test running multiple clients `./sendTest.sh <fileName> <IP-address>:<port-number> <number-of-clients> [bufSize]`

* To test transfers in different environments `./environmentTest.sh <fileName> <server-hostname>:<port-number> <bufferSize> <numTransfers>`

## Limitations

## Author Information
Lucas Sudol
lsudol@uoguelph.ca
1227984

Sebastian Kula
skula@uoguelph.ca
1247993

## Development History

## Acknowledgments


