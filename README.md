# CaveEngine

Yet another game engine.

## Prerequisites

### Windows
```shell
$ sh scripts/build_assimp.sh
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

### WASM
```shell
$ source /path/to/emsdk/emsdk_env.sh
$ mkdir build-emscripten
$ cd build-emscripten
$ emcmake cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXE_LINKER_FLAGS="-sUSE_GLFW=3 -sUSE_WEBGL2=1 -sFULL_ES3=1 -sPTHREAD_POOL_SIZE=16"
$ mingw32-make
$ cd ../scripts
$ python -m simple-http-server.py
```

### Profiling the Engine
Open `bin/Optick.exe` to start profiling session

## Screenshots

<p align="center">
    <img src="https://github.com/Guo-Haowei/GameEngine/blob/master/documents/editor.png" width="70%">
</p>
<p align="center">
    <em>Showcase — Sponza (VXGI)</em>
</p>

<p align="center">
    <img src="https://github.com/Guo-Haowei/GameEngine/blob/master/documents/path_tracer.png" width="70%">
</p>
<p align="center">
    <em>Showcase — Sponza (Path traced)</em>
</p>

<p align="center">
    <img src="https://github.com/Guo-Haowei/GameEngine/blob/master/documents/breakfast_room.png" width="70%">
</p>
<p align="center">
    <em>Showcase — Breakfast Room</em>
</p>

<p align="center">
    <img src="https://github.com/Guo-Haowei/GameEngine/blob/master/documents/tile_map_editor.gif" width="70%">
</p>
<p align="center">
    <em>Showcase — Tile Map Editor</em>
</p>

## Graphics APIs

API           | Implementation
--------------|----------------------
OpenGL        | Done
Direct3D 11   | Done
Direct3D 12   | Done
Vulkan        | WIP
Metal         | WIP
