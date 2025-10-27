workspace "NES_Emulator"
    configurations { "Debug", "Release" }
    architecture "x86_64"
    language "C"

project "NES_Emulator"
    kind "ConsoleApp"
    targetdir "bin/%{cfg.buildcfg}"
    files { "src/**.c", "src/**.h" }

    buildoptions { "-Wall", "-Werror" }

    -- Header files
    includedirs { "lib/SDL2/include/SDL2" }

    -- Library folder
    libdirs { "lib/SDL2/lib" }

    links { "SDL2" }

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
