Game core made using OpenGL, supposed to be first person target shooter.

It uses GLFW3 and OpenGL 3.3 on desktops and OpenGLES 2.0 for the web build.

## Build
The only official way to build this project is using [Zig](https://ziglang.org/).
There is also an experimental CMake support, but I don't recommend it. Currently tested only on Windows with [ninja](https://ninja-build.org/) build system.

Other method might be compiling everything yourself with `gcc` or `clang`, but that requires linking all needed libraries by yourself.

### Building with Zig
Use [Zig 0.13.0](https://ziglang.org/download/).

To just run the project:
```console
zig build run
```

To create the executable into `zig-out/bin/` directory:
```console
zig build
```
To further run this executable, just place it in the root folder of this repository.

### Dependencies
The only dependency (other than OpenGL) is `GLFW3`, please use version 3.4 or newer.

If you get `error: unable to find dynamic system library 'glfw3'` error message, then the build system was not able to find GLFW. You can add it manually by changing the build file. On top of `build.zig` file there are constant definitions:
```zig
// GLFW custom paths
const glfw_lib_dir_path: ?String = null;
const glfw_include_dir_path: ?String = null;
```
Just change those to match your glfw location, for example:
```zig
// GLFW custom paths
const glfw_lib_dir_path: ?String = "path/to/your/glfw/3.4/lib";
const glfw_include_dir_path: ?String = "path/to/your/glfw/3.4/include";
```

Similiarily you might get `error: 'GLFW/glfw3.h' file not found` error message, it means that the build system didn't find GLFW3 include directory. Fix this by the same procedure described above.

### Building with CMake (Experimental)
Requires CMake version 3.12 or higher. If you get into dependency issues you will have to modify the `CMakeLists.txt` yourself.

Create the build folder:
```sh
mkdir cmake-build
cd cmake-build
```
Generate build files and run the build system:
```sh
cmake ..
cmake --build .
```
Resulting binary gets written into `build/bin` folder.

## Web build
This project is structured in a way that it is possible to compile it for the web (wasm with WebGL).
However the rendering in the web build is still broken, so I don't recommend it.

You need to have [emscripten](https://emscripten.org/) toolchain installed and accessible through path.
You then use:
```console
zig build -Dtarget=wasm32-emscripten
```
Note that you must first create the `zig-out/web` folder, where the web version gets created.
Next you can run it using any http server, for example with python3:
```console
python3 -m http.server 8080 -d ./zig-out/web/
```
After that you open it in your browser (`localhost:8080/shooting_practice.html`).
