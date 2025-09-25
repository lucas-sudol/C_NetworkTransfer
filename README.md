# Project Title
A3 - Network Transfer Using C

## Description
Server Usage: %s <port-number> [bufSize]
Client Usage: %s <fileName> <IP-address/hostname>:<port-number> [bufSize]

Provides functionality for sending text files from a client to a server using TCP.
Supports host name resolution.

Compile program on server and client machines using make all

## Getting Started

### Executing program
* Build the program with `make all`

* Start the server `./server port-number bufSize`

* Send a file using the client `./sendFile fileName IP-address:port-number bufSize`

* To test running multiple clients `./sendTest.sh <fileName> <IP-address>:<port-number> <number-of-clients> [bufSize]`

* To test transfers in different environments `python3 environmentTest.py <fileName> <server-hostname>:<port-number> <bufferSize> <numTransfers>`
    * must have server running on different computer to effectivley test environment

## Author Information
Lucas Sudol
Sebastian Kula




