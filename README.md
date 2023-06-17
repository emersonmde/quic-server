# quic-server
Exploring how to create a Quic server in C

## Building

Clone this repo and the submodules, then build:
```
git clone --recurse-submodules https://github.com/emersonmde/quic-server.git

# Or to initialize submodules in parallel:
# git clone --recurse-submodules -j10 https://github.com/emersonmde/quic-server.git

mkdir quic-server/build
cd quic-server/build
cmake ../ && make -j
```

Then run the server:
```
./quic-server
```
