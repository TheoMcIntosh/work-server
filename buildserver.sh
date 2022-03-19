rm server

echo "Removed old server, building server."
g++ -std=c++17 -mcpu=generic+crypto MinerServer.cpp lib/sha256-armv8-aarch64.S -o server -O2 -pthread
echo "Server built."
./server
