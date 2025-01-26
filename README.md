Game core made using OpenGL, supposed to be first person target shooter.

#### Compiling
There is no proper build option yet, I plan to add zig build file in the future.

Currently on Windows I just compile with:
```
g++ *.cpp *.c -std=c++17 -Iinclude -lglfw3 -lopengl32 -lgdi32
```
You probably would need to modify it for yourself. Also it's necessary to install GLFW3 + include it's headers.
_It's probably a good idea to separate .cpp and .c file compilation._
