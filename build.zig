const std = @import("std");

const String = [:0]const u8;

// Public constants related to build configuration.
// Please keep list of cpp and c files updated with all file names that need to get compiled.
pub const project_name = "shooting_practice";
pub const version_string = "v0.2";

pub const cpp_files = [_]String{ "collision.cpp", "drawing.cpp", "game.cpp", "lighting.cpp", "loop_data.cpp", "main-game.cpp",
                                 "main-test.cpp", "main.cpp", "meshes.cpp", "mouse_manager.cpp", "movement.cpp", "shaders.cpp",
                                 "shared_gl_context.cpp", "textures.cpp", "ui.cpp", "utils.cpp", "window_manager.cpp" };
pub const c_files = [_]String{ "cgltf.c", "glad.c", "nuklear.c", "stb_image.c", "tinyobj_loader_c.c" };

pub const cpp_std_ver = "c++17";
pub const c_std_ver = "c99"; // good idea to use c99 or newer, GLFW 3.4 seems to require at least c99

//TODO glfw add override from command line
// GLFW custom paths
const glfw_lib_dir_path: ?String = null;
const glfw_include_dir_path: ?String = null;

// Python web server settings
const python_cmd_name = "python";
const python_http_port = 8080;


pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    switch (target.result.os.tag)
    {
        .emscripten =>
        {
            std.log.info("Compiling for the web using emscripten!", .{});

            const web_out_dir = "zig-out/web/"; //TODO create this directory if it does not exist yet
            const local_em_dir = "emscripten-stuff/";
            const shell_file = "shell.html";

            const emcc_arguments = [_]String{
                "em++",
                // "--use-port=contrib.glfw3",
                "-o", web_out_dir ++ project_name ++ ".html",   //output
            } ++ cpp_files ++ c_files ++ [_]String{
                "-Os", "-O3", "-Wall",
                "-I.", "-I./include",                           //includes (header files)
                //flags and other parameters:
                "-sUSE_GLFW=3",
                "-sWASM=1",
                "-sALLOW_MEMORY_GROWTH=1", "-sSTACK_SIZE=20MB",
                "-sWASM_MEM_MAX=3GB", "-sTOTAL_MEMORY=3GB",
                "-sABORTING_MALLOC=0", 
                "-sFORCE_FILESYSTEM=1", 
                "-sASSERTIONS=1", 
                "-sERROR_ON_UNDEFINED_SYMBOLS=0", //TODO check this
                "-sEXPORTED_FUNCTIONS=['_malloc','_free','_main', '_emsc_set_window_size']",
                "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap",
                "-sUSE_OFFSET_CONVERTER",
                // "--use-preload-cache", //TODO check this
                //preloaded files:
                "--preload-file", "assets",
                "--preload-file", "shaders",
                //shell:
                "--shell-file", local_em_dir ++ shell_file, 
                //C compiler parameters (???):
                "-DPLATFORM_WEB", 
                "-DVERSION_STRING=\"" ++ version_string ++ "\"", // adding quatation marks so that the macro value is a string literal
                "-Os", "-O3",
            };

            const emcc = b.addSystemCommand(&emcc_arguments);
            b.getInstallStep().dependOn(&emcc.step);

            // python https server script when run command specified
            const python_http_port_string = std.fmt.comptimePrint("{d}", .{ python_http_port });
            const python_http_arguments = [_]String{
                python_cmd_name,
                "-m", "http.server", python_http_port_string,
                "-d", web_out_dir,
            };

            const python_http_run = b.addSystemCommand(&python_http_arguments);
            python_http_run.step.dependOn(b.getInstallStep());
            var run_step = b.step("run", "run " ++ project_name);
            run_step.dependOn(&python_http_run.step);
        },
        else =>
        {
            //game executable
            const exe = b.addExecutable(.{ .name = project_name, .target = target, .optimize = optimize });

            exe.defineCMacro("BUILD_OPENGL_330_CORE", null);
            exe.defineCMacro("VERSION_STRING", "\"" ++ version_string ++ "\""); // adding quatation marks so that the macro value is a string literal

            exe.addLibraryPath(.{ .src_path = .{ .owner = b, .sub_path = "lib" } });
            exe.addIncludePath(.{ .src_path = .{ .owner = b, .sub_path = "include" } });
            exe.linkLibCpp();

            if (glfw_lib_dir_path) |lib_path|
            {
                exe.addLibraryPath(.{ .cwd_relative = lib_path });
            }
            if (glfw_include_dir_path) |include_path|
            {
                exe.addIncludePath(.{ .cwd_relative = include_path });
            }

            exe.addCSourceFiles(.{ .files = &cpp_files, .flags = &.{ "-std=" ++ cpp_std_ver } });
            exe.addCSourceFiles(.{ .files = &c_files, .flags = &.{ "-std=" ++ c_std_ver } });

            switch (target.result.os.tag)
            {
                .macos => {
                    exe.linkSystemLibrary("glfw3");

                    exe.linkFramework("Foundation");
                    exe.linkFramework("Cocoa");
                    exe.linkFramework("OpenGL");
                    // exe.linkFramework("CoreAudio");
                    // exe.linkFramework("CoreVideo");
                    exe.linkFramework("IOKit");
                },
                //TODO correct linux libraries
                .linux => {
                    exe.linkSystemLibrary("glfw3"); //TODO check this

                    exe.addLibraryPath(.{ .cwd_relative = "/usr/lib64/" });
                    exe.linkSystemLibrary("GL");
                    exe.linkSystemLibrary("rt");
                    exe.linkSystemLibrary("dl");
                    exe.linkSystemLibrary("m");
                    exe.linkSystemLibrary("X11");
                },
                else => {
                    exe.linkSystemLibrary("glfw3");

                    exe.linkSystemLibrary("opengl32");
                    exe.linkSystemLibrary("gdi32");
                },
            }

            const run_cmd = std.Build.addRunArtifact(b, exe);
            var run_step = b.step("run", "run " ++ project_name);
            run_step.dependOn(&run_cmd.step);

            b.installArtifact(exe);
        },
    }
}
