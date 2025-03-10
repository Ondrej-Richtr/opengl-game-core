const std = @import("std");

const String = [:0]const u8;

pub const project_name = "shooting_practice";

pub const cpp_files = [_]String{ "collision.cpp", "drawing.cpp", "game.cpp", "lighting.cpp", "main-game.cpp",
                                 "main-test.cpp", "main.cpp", "meshes.cpp", "movement.cpp", "shaders.cpp",
                                 "textures.cpp", "ui.cpp", "utils.cpp", "window_manager.cpp" };
pub const c_files = [_]String{ "glad.c", "nuklear.c", "stb_image.c" };

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    //TODO this
    // const main_location = std.Build.LazyPath{ .src_path = .{ .owner = b, .sub_path = "main.cpp" } };

    switch (target.result.os.tag)
    {
        .emscripten => //TODO web build
        {
            // std.log.info("Compiling for the web using emscripten!", .{});

            // if (b.sysroot == null) {
            //     @panic("Pass '--sysroot \"$EMSDK/upstream/emscripten\"'");
            // }

            // const emscripten_include_dir = try std.fs.path.join(b.allocator, &.{ b.sysroot.?, "cache", "sysroot", "include" });
            // defer b.allocator.free(emscripten_include_dir);

            // var lib = b.addStaticLibrary(.{ .name = project_name, .root_source_file = main_location,
            //                                   .target = target, .optimize = optimize, });
            // lib.entry = std.Build.Step.Compile.Entry.disabled;
            // lib.linkLibC();
            // lib.pie = true;
            // switch (optimize)
            // {
            //     .Debug, .ReleaseSafe => lib.bundle_compiler_rt = true,
            //     .ReleaseFast, .ReleaseSmall => {},
            // }
            // lib.addIncludePath(.{ .cwd_relative = emscripten_include_dir });

            // raylib.addTo2(b, lib, target, optimize, raylib_location);

            // const lib_out_dir = "zig-out/lib/";
            // const web_out_dir = "zig-out/web/";
            // const local_em_dir = "emscripten-stuff/";

            // const emcc = b.addSystemCommand(&.{
            //     "emcc",
            //     "-o", web_out_dir ++ project_name ++ ".html",   //output
            //     //local_em_dir ++ "entry.c", "raylib/marshal.c",  //compiled c files (entry.c and raylib binding marshal.c)
            //     "raylib/marshal.c",  //compiled c files - raylib marshal.c bindings
            //     lib_out_dir ++ "lib" ++ project_name ++ ".a",            //library file made in previous step
            //     "-Os", "-O3", "-Wall",
            //     "-I.", "-I./raylib", "-I" ++ raylib_location,   //includes (header files)
            //     "-L.", "-L" ++ lib_out_dir,                     //used libraries (.a files)
            //     "-lraylib", "-l" ++ project_name,               //using raylib and lib file from previous step
            //     //flags and other parameters:
            //     "-sUSE_GLFW=3",
            //     "-sASYNCIFY=1", "-sASYNCIFY_STACK_SIZE=512000000", "-sASYNCIFY_DEBUG=1", "-sASYNCIFY_IMPORTS=_asyncify_stop_unwind", //TODO stack size and other stuff
            //     "-sWASM=1",
            //     "-sALLOW_MEMORY_GROWTH=1",
            //     "-sWASM_MEM_MAX=3GB", "-sTOTAL_MEMORY=3GB",
            //     "-sABORTING_MALLOC=0", 
            //     "-sFORCE_FILESYSTEM=1", 
            //     "-sASSERTIONS=1", 
            //     "-sERROR_ON_UNDEFINED_SYMBOLS=0", //TODO check this
            //     //"-sEXPORTED_FUNCTIONS=['_malloc','_free','_main', '_emsc_main','_emsc_set_window_size']",
            //     "-sEXPORTED_FUNCTIONS=['_malloc','_free','_main', '_emsc_set_window_size']",
            //     "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap",
            //     "-sUSE_OFFSET_CONVERTER",
            //     //preloaded files:
            //     "--preload-file", "assets",
            //     "--preload-file", "shaders",
            //     //shell:
            //     "--shell-file", local_em_dir ++ "minshell.html", //TODO shell file 
            //     //C compiler parameters (???):
            //     "-DPLATFORM_WEB", 
            //     "-Os", "-O3",
            // });


            // b.installArtifact(lib);
            // emcc.step.dependOn(&lib.step);
            // b.getInstallStep().dependOn(&emcc.step);
        },
        else =>
        {
            //game executable
            const exe = b.addExecutable(.{ .name = project_name, .target = target, .optimize = optimize });
            exe.addIncludePath(.{ .src_path = .{ .owner = b, .sub_path = "include" } });
            exe.linkLibCpp();

            exe.addCSourceFiles(.{ .files = &cpp_files, .flags = &.{ "-std=c++17" } });
            exe.addCSourceFiles(.{ .files = &c_files, .flags = &.{ "-std=c99" } });

            switch (target.result.os.tag)
            {
                //TODO correct macOS frameworks
                .macos => {
                    const glfw_lib_dir_path = "/opt/homebrew/Cellar/glfw/3.4/lib";
                    const glfw_include_dir_path = "/opt/homebrew/Cellar/glfw/3.4/include";

                    exe.addLibraryPath(.{ .cwd_relative = glfw_lib_dir_path });
                    exe.addIncludePath(.{ .cwd_relative = glfw_include_dir_path });
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
                    exe.addLibraryPath(.{ .cwd_relative = "/usr/lib64/" });
                    exe.linkSystemLibrary("GL");
                    exe.linkSystemLibrary("rt");
                    exe.linkSystemLibrary("dl");
                    exe.linkSystemLibrary("m");
                    exe.linkSystemLibrary("X11");
                },
                else => {
                    //TODO this is not part of the repo - make glfw compile from scratch?
                    const glfw_lib_dir_path = "lib";

                    exe.addLibraryPath(.{ .cwd_relative = glfw_lib_dir_path });
                    exe.linkSystemLibrary("glfw3");

                    exe.linkSystemLibrary("opengl32");
                    exe.linkSystemLibrary("gdi32");
                },
            }

            const run_cmd = std.Build.addRunArtifact(b, exe);
            var run_step = b.step("run", "run " ++ project_name);
            run_step.dependOn(&run_cmd.step);

            b.installArtifact(exe);

            //TODO is check command useful for C++ project?
            //check
            // const exe_check = b.addExecutable(.{ .name = project_name, .root_source_file = main_location, .target = target, .optimize = optimize });

            // const check = b.step("check", "Check if " ++ project_name ++ " compiles");
            // check.dependOn(&exe_check.step);
        },
    }
    
}
