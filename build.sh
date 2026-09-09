mkdir -p ./build/
g++ src/main.cpp -o build/main --std=c++23 $(pkg-config --cflags --libs raylib)
