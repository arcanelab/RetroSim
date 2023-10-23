#!/bin/bash

# Check if build/SDL-gpu/lib/libSDL2_gpu.a exists
if [ -f src/extern/sdl-gpu/build/SDL_gpu/lib/libSDL2_gpu.a ]; then
    echo "SDL_gpu already compiled."
    exit 0
fi

echo "Compiling SDL_gpu..."
ls -l

# Navigate to the specified directory
mkdir -p ./src/extern/sdl-gpu/build
cd ./src/extern/sdl-gpu/build

# Check if cd command was successful
if [ $? -ne 0 ]; then
    echo "Error: Couldn't change to the desired directory."
    exit 1
fi

# Run cmake
cmake .. -G "Unix Makefiles" -DDEFAULT_BUILD_SHARED=OFF -DBUILD_FRAMEWORK=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build . --target SDL_gpu

cd ../../../../

# Check if cmake command was successful
if [ $? -ne 0 ]; then
    echo "Error: cmake failed."
    exit 1
fi

echo "cmake ran successfully!"
