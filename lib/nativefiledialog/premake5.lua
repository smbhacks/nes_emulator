project "nativefiledialog"
    kind "StaticLib"
    language "C++"

    location "build"
    objdir "build/obj/%{cfg.platform}/%{cfg.buildcfg}"
    targetdir "build/bin/%{cfg.platform}/%{cfg.buildcfg}"
    
    includedirs {
        "submodule/src/include"
    }

    files {
        "submodule/src/*.h",
        "submodule/include/*.h",
        "submodule/src/nfd_common.c"
    }

    filter "system:windows"
        links { "comctl32" }   -- NFD requires this
        files {
            "submodule/src/nfd_win.cpp"
        }

    filter "system:linux"
        files {
            "submodule/src/nfd_gtk.c"
        }