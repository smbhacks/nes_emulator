workspace "NES_Emulator"
    configurations { "Debug", "Release" }
    architecture "x86_64"
    language "C"

project "NES_Emulator"
    kind "ConsoleApp"
    targetdir "bin/%{cfg.buildcfg}"
    files { "src/**.c", "src/**.h" }

    files {
        "lib/gl3w/gl3w.c"
    }
    includedirs {
        "lib/gl3w"
    }

    ---------------------------------------
    -- cimgui + imgui
    ---------------------------------------
    files {
        "lib/cimgui/cimgui.cpp",
        "lib/cimgui/imgui/*.cpp",
--        "lib/cimgui/imgui/backends/imgui_impl_glfw.cpp",
        "lib/cimgui/imgui/backends/imgui_impl_opengl3.cpp",
        "lib/cimgui/imgui/backends/imgui_impl_sdl2.cpp"
    }

    includedirs {
        "lib/cimgui",
        "lib/cimgui/imgui",
        "lib/cimgui/imgui/backends"
    }

    filter {
        "files:lib/cimgui/**.cpp",
        "files:lib/cimgui/imgui/**.cpp",
        "files:lib/cimgui/imgui/backends/**.cpp"
    }
        language "C++"
    filter {}

    ---------------------------------------
    -- SDL2
    ---------------------------------------
    includedirs { "lib/SDL2/include/SDL2" }
    libdirs { "lib/SDL2/lib" }


    ---------------------------------------
    -- gmake (Linux build)
    ---------------------------------------
    filter "action:gmake*"
        buildoptions { "-Wall", "-Werror" }
        links { "SDL2" }

        -- IMPORTANT: link with the C++ runtime
        linkoptions { "-lstdc++" }


    ---------------------------------------
    -- Windows (VS2022)
    ---------------------------------------
    filter "action:vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" }
        buildoptions { "/W3", "/WX" }
        links { "SDL2-static.lib", "SDL2main.lib" }

        -- Required for linking C++ objects into a C project
        links { "msvcprt.lib", "msvcp140.lib" }


    filter "system:windows"
        defines { "SDL_STATIC", "SDL_MAIN_HANDLED" }
        links {
            "user32", "gdi32", "winmm", "imm32", "ole32", "oleaut32",
            "version", "uuid", "setupapi"
        }

    filter "system:linux"
        defines { "SDL_STATIC" }
        links {
            "pthread", "dl", "m"
        }

    filter "system:macosx"
        defines { "SDL_STATIC" }
        links {
            "iconv"
        }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
