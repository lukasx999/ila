run: build
    ./build/ila

configure:
    cmake -Bbuild -GNinja

build: configure
    cmake --build build
