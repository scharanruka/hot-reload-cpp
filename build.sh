rm -rf ./build/
mkdir -p ./build/


g++ -shared -fPIC src/sim.cpp -o build/libsim.so --std=c++23 $(pkg-config --cflags --libs raylib)
g++ src/main.cpp -o build/main --std=c++23 $(pkg-config --cflags --libs raylib) -ldl
