FROM debian:buster-slim

RUN apt update && apt install -y lua5.3 lua5.3-dev g++ make cmake

WORKDIR /app

COPY CMakeLists.txt .
COPY LICENSE .
COPY include/ include/
COPY test/ test/

RUN mkdir build ; cd build ; cmake ../ ; cmake --build . ; chmod a+x bin/reconduit_test

ENTRYPOINT ["build/bin/reconduit_test"]
