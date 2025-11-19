workspace "NES_Emulator"
    configurations { "Debug", "Release" }
    architecture "x86_64"
    language "C"

include "lib/cimgui"

project "NES_Emulator"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"

    files { 
        "src/**.c", 
        "src/**.h",
        "lib/gl3w/src/gl3w.c"
    }

    includedirs {
        "lib/SDL2/include/SDL2",
        "lib/cimgui/submodule",
        "lib/cimgui/submodule/imgui",
        "lib/cimgui/submodule/imgui/backends",
        "lib/gl3w/include"
    }

    libdirs {
        "lib/SDL2/lib",
        "lib/cimgui/submodule/build/bin/%{cfg.platform}/%{cfg.buildcfg}"
    }

    links { 
        "cimgui", 
        "SDL2"
    }

    filter "system:windows"
        defines { "SDL_STATIC", "SDL_MAIN_HANDLED" }
        links { "user32", "gdi32", "winmm", "imm32", "ole32", "oleaut32", "version", "uuid", "setupapi", "opengl32" }

    filter "system:linux"
        defines { "SDL_STATIC" }
        links { "pthread", "dl", "m", "GL", "stdc++" }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"

    filter "action:gmake*"
        buildoptions { "-Wall", "-Werror" }
