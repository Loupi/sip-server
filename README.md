# sip-server
sip-server parses an SIP registration dump, and starts an asynchronous tcp server
for clients to query the sip register by sending AOR identifiers, line by line.
When found, the SIP json is returned to the client. If not, an empty line is returned.
If clients are inactive for x milliseconds they are disconnected.

## Usage
sip-server [s] [-b bind_address] [-p port] [-t timeout(ms)] [-q queue_len] -i input_file
ie: sip-server -b 0.0.0.0 -p 8080 -t 10000 -q 128 -i ../regs

## Defaults
* verbose
* bind_address: 0.0.0.0
* port: 8080
* timeout: 10000
* queue_len: 128

## Dependencies
* libuv for asynchronous network i/o
* plibsys for hastable implementation
* getopt for command line parsing


## Build Instructions

### Requirements
Conan package manager and CMake are required to build the project.
Visit their website for installation instructions for your system.
Set your path environment variable accordingly.
* https://conan.io/downloads.html
* https://cmake.org/


### All
On all systems, extract the sip-server archive where you prefer and cd to its directory.

### Linux
```
mkdir build
cd build
conan install ..
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
./bin/sip-server -i ../regs
```

### Windows
```
mkdir win32
cd win32
conan install .. -s compiler="Visual Studio" -s compiler.version=14 -s arch=x86_64 -s build_type=Release
cmake .. -G "Visual Studio 14 Win64"
cmake --build .
cd bin
sip-server.exe -i ../../regs
sip-server.exe -b 0.0.0.0 -p 7070 -i ../../regs
```

## Server Dump
```
Server started 0.0.0.0:8080
127.0.0.1:59500 connected.
127.0.0.1:59500 requested 7tq72ZFDLfSmhsfkmTzyaDbC27YNeP Found!
127.0.0.1:59500 requested 01589282e9ec182e9e000100620002 Found!
127.0.0.1:59500 requested 0151ca8089adfe0d88000100620005 Found!
127.0.0.1:59500 requested 0ZiceAybQoYuwTU4bXHHS660pmZaHT Found!
127.0.0.1:59500 requested 01591d1efa4fc65f44000100620002 Found!
127.0.0.1:59500 requested 0155e0df190b7fec6a000100610002 Found!
127.0.0.1:59500 requested 01461a9ba857ca4435000100620005 Found!
127.0.0.1:59500 requested 015729b06ab825c7c3000100620007 Found!
127.0.0.1:59500 requested 0157afae3f759a99d7000100620002 Found!
127.0.0.1:59500 requested 01531096fd8516dff9000100620007 Found!
127.0.0.1:59500 requested 01565c40a7b3da331a000100620007 Found!
127.0.0.1:59500 requested 01565c40a7e8dbe31a000100620007 Found!
127.0.0.1:59500 requested 0152fb6936ae1f3fa6000100620009 Found!
127.0.0.1:59500 requested 19lE2e06bPJgZVkyZdmdUt1bRthCiy Found!
127.0.0.1:59500 requested 0155cc14d8ff7a2b5e000100620007 Found!
127.0.0.1:59500 requested 0154705be976f837f4000100620009 Found!
127.0.0.1:59500 requested 01493508f2e350e063000100620005 Found!
127.0.0.1:59500 requested 0145519590b49a1670000100620002 Found!
127.0.0.1:59500 requested 01584173ba44194a6a000100620002 Found!
127.0.0.1:59500 requested 5iRFVassRmPlaVuTBTA3qPvWh6Qko7 Found!
127.0.0.1:59500 requested 7tq72ZFDLfSmhsfkmTzyaDbC27YNeP Found!
127.0.0.1:59500 requested 01589282e9ec182e9e000100620002 Found!
127.0.0.1:59500 requested 0151ca8089adfe0d88000100620005 Found!
127.0.0.1:59500 requested 0ZiceAybQoYuwTU4bXHHS660pmZaHT Found!
127.0.0.1:59500 requested 01591d1efa4fc65f44000100620002 Found!
127.0.0.1:59500 requested 0155e0df190b7fec6a000100610002 Found!
127.0.0.1:59500 requested 01461a9ba857ca4435000100620005 Found!
127.0.0.1:59500 requested 015729b06ab825c7c3000100620007 Found!
127.0.0.1:59500 requested 0157afae3f759a99d7000100620002 Found!
127.0.0.1:59500 requested 01531096fd8516dff9000100620007 Found!
127.0.0.1:59500 requested ssd Not found!
127.0.0.1:59500 requested sd Not found!
127.0.0.1:59500 requested sd Not found!
127.0.0.1:59500 timeout.
127.0.0.1:59502 connected.
127.0.0.1:59502 requested 01531096fd8516dff9000100620007 Found!
127.0.0.1:59502 requested 01565c40a7b3da331a000100620007 Found!
127.0.0.1:59502 timeout.
```