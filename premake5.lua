workspace "NES_Emulator"
    configurations { "Debug", "Release" }
    architecture "x86_64"
    language "C"

project "NES_Emulator"
    kind "ConsoleApp"
    targetdir "bin/%{cfg.buildcfg}"
    files { "src/**.c", "src/**.h" }

    -- Header files
    includedirs { "lib/SDL2/include/SDL2" }

    -- Library folder
    libdirs { "lib/SDL2/lib" }

    filter "action:gmake*"
        buildoptions { "-Wall", "-Werror" }
	links { "SDL2" }

    filter "action:vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" }
        buildoptions { "/W3", "/WX" }
	links { "SDL2-static.lib", "SDL2main.lib" }

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
