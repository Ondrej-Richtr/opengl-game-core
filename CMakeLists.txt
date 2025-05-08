#TODO check for usable versions
cmake_minimum_required(VERSION 3.12)

#keep this up to date with build.zig
project(shooting_practice LANGUAGES C CXX)

#keep this up to date with build.zig
set(version_string "v0.2")

list(APPEND cpp_files "collision.cpp" "drawing.cpp" "game.cpp" "lighting.cpp" "main-game.cpp"
                      "main-test.cpp" "main.cpp" "meshes.cpp" "movement.cpp" "shaders.cpp"
                      "shared_gl_context.cpp" "textures.cpp" "ui.cpp" "utils.cpp" "window_manager.cpp")
list(APPEND c_files   "cgltf.c" "glad.c" "nuklear.c" "stb_image.c" "tinyobj_loader_c.c")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(shooting_practice ${cpp_files} ${c_files})

target_compile_features(shooting_practice PUBLIC cxx_std_17)
target_compile_features(shooting_practice PUBLIC c_std_99)

add_compile_definitions(BUILD_OPENGL_330_CORE)
add_compile_definitions(VERSION_STRING="${version_string}")

target_link_libraries(shooting_practice "-L${PROJECT_SOURCE_DIR}/lib")
target_include_directories(shooting_practice PUBLIC ./include)

target_link_libraries(shooting_practice -lglfw3)

#TOOD add other systems as well
IF (WIN32)
    target_link_libraries(shooting_practice -lopengl32)
    target_link_libraries(shooting_practice -lgdi32)
ENDIF()
