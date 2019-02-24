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
02/24/2019 00:29:50 0.0.0.0:8080 server started.
02/24/2019 00:29:55 127.0.0.1:59070 connected.
02/24/2019 00:29:58 127.0.0.1:59070 requested test Not found!
02/24/2019 00:30:00 127.0.0.1:59070 requested 1212 Not found!
02/24/2019 00:30:01 127.0.0.1:59070 requested  Not found!
02/24/2019 00:30:11 127.0.0.1:59070 timeout.
02/24/2019 00:30:21 127.0.0.1:59072 connected.
02/24/2019 00:30:24 127.0.0.1:59072 requested 015043a61826b39eeb000100620005 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 01577bc1415ed95760000100620002 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 01499acbbaaba2c163000100620005 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 015620b05b29ce6994000100620007 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 015043a61826b39eeb000100620005 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 01577bc1415ed95760000100620002 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 01499acbbaaba2c163000100620005 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 015620b05b29ce6994000100620007 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 015043a61826b39eeb000100620005 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 01577bc1415ed95760000100620002 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 01499acbbaaba2c163000100620005 Found!
02/24/2019 00:30:24 127.0.0.1:59072 requested 015620b05b29ce6994000100620007 Found!
02/24/2019 00:30:34 127.0.0.1:59072 timeout.
02/24/2019 00:30:48 127.0.0.1:59074 connected.
02/24/2019 00:30:50 127.0.0.1:59074 requested 015620b05b29ce6994000100620007 Found!
02/24/2019 00:30:50 127.0.0.1:59074 requested 015043a61826b39eeb000100620005 Found!
02/24/2019 00:30:50 127.0.0.1:59074 requested 01577bc1415ed95760000100620002 Found!
02/24/2019 00:30:50 127.0.0.1:59074 requested  Not found!
02/24/2019 00:31:00 127.0.0.1:59074 timeout.
02/24/2019 00:31:34 127.0.0.1:59076 connected.
02/24/2019 00:31:38 127.0.0.1:59076 requested allo Not found!
02/24/2019 00:31:38 127.0.0.1:59076 closed.
^C02/24/2019 00:31:41 Shutting down...
```

## Netcat Client Dump
```
015620b05b29ce6994000100620007
015043a61826b39eeb000100620005
01577bc1415ed95760000100620002

{"addressOfRecord":"015620b05b29ce6994000100620007","tenantId":"0127d974-f9f3-0704-2dee-000100420001","uri":"sip:015620b05b29ce6994000100620007@212.96.24.153;jbcuser=cpe70","contact":"<sip:015620b05b29ce6994000100620007@195.244.33.112;jbcuser=cpe70>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\"","path":["<sip:Mi0xOTkuMTkyLjE2NS4xOTQtMTk2MjI@50.31.215.193:5060;lr>"],"source":"203.31.128.190:19622","target":"81.52.1.182:5061","userAgent":"polycom.vvx.500","rawUserAgent":"PolycomVVX-VVX_500-UA/137.37.167.11","created":"2016-12-13T14:22:44.340Z","lineId":"015620b0-5b19-5cdc-b994-000100620007"}
{"addressOfRecord":"015043a61826b39eeb000100620005","tenantId":"0127d974-f9f3-0704-2dee-000100420001","uri":"sip:015043a61826b39eeb000100620005@160.255.34.219:5082","contact":"<sip:015043a61826b39eeb000100620005@129.179.163.119:5082>","path":["<sip:Mi03My4zLjEyNy4xMjQtNTA4Mg@215.215.93.114:5060;lr>"],"source":"253.207.240.103:5082","target":"105.57.250.160:5061","userAgent":null,"rawUserAgent":"Vtech Vesa VSP735 36.185.137.145-0","created":"2016-12-17T16:11:04.881Z","lineId":"0143e3f4-2979-ea68-b5e8-000100620002"}
{"addressOfRecord":"01577bc1415ed95760000100620002","tenantId":"0127d974-f9f3-0704-2dee-000100420001","uri":"sip:01577bc1415ed95760000100620002@228.138.99.123;jbcuser=cpe70","contact":"<sip:01577bc1415ed95760000100620002@46.164.103.17;jbcuser=cpe70>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\"","path":["<sip:Mi0xOTkuMTkyLjE2NS4xOTQtMTk2MjI@8.38.197.27:5060;lr>"],"source":"11.202.129.80:19622","target":"80.26.173.222:5061","userAgent":"polycom.vvx.500","rawUserAgent":"PolycomVVX-VVX_500-UA/204.12.200.4","created":"2016-12-13T04:44:28.484Z","lineId":"0141aad9-b53a-529c-e8cb-000100620005"}

```