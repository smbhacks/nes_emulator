project "cimgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"

    location "build"
    objdir "build/obj/%{cfg.platform}/%{cfg.buildcfg}"
    targetdir "build/bin/%{cfg.platform}/%{cfg.buildcfg}"

    includedirs {
        "submodule/imgui",
        "submodule/imgui/backends",
        "../SDL2/include/SDL2",
        "../gl3w/include"
    }

    files {
        "submodule/cimgui.cpp",
        "submodule/imgui/*.cpp",
        "submodule/imgui/backends/imgui_impl_sdl2.cpp",
        "submodule/imgui/backends/imgui_impl_opengl3.cpp"
    }

    defines {
        "IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1",
        "IMGUI_IMPL_OPENGL_LOADER_GL3W"
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"
        defines { "IMGUI_IMPL_API=extern \"C\" __declspec(dllexport)" }
        links { "SDL2-static", "opengl32" }

    filter "system:linux"
        defines { "IMGUI_IMPL_API=extern \"C\"" }
        links { "SDL2", "GL", "pthread", "dl", "m" }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"