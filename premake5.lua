workspace "sockets"
    configurations {"Debug", "Release"}
    location "build"

project "client"
    kind "ConsoleApp"
    language "C++"
    architecture "x64"
    cppdialect "c++latest"
    staticruntime "On"
    targetdir "bin/%{cfg.buildcfg}"

files {"src/client.cpp"}
includedirs {"include"}

defines {"SFML_STATIC"}

-- sfml deps {in windows}
links {"opengl32.lib"}
links {"winmm.lib"}
links {"ws2_32.lib"}

-- sfml deps {ext}
links {"lib/flac.lib"}
links {"lib/freetype.lib"}
links {"lib/openal32.lib"}
links {"lib/vorbis.lib"}
links {"lib/vorbisenc.lib"}
links {"lib/vorbisfile.lib"}

filter "configurations:Debug"
    runtime "Debug"
    defines {"DEBUG"}
    symbols "On"
    links {"lib/Debug/sfml-audio-s-d.lib"}
    links {"lib/Debug/sfml-graphics-s-d.lib"}
    links {"lib/Debug/sfml-network-s-d.lib"}
    links {"lib/Debug/sfml-system-s-d.lib"}
    links {"lib/Debug/sfml-window-s-d.lib"}

filter "configurations:Release"
    runtime "Release"
    defines {"NDEBUG"}
    optimize "On"
    links {"lib/Release/sfml-audio-s.lib"}
    links {"lib/Release/sfml-graphics-s.lib"}
    links {"lib/Release/sfml-network-s.lib"}
    links {"lib/Release/sfml-system-s.lib"}
    links {"lib/Release/sfml-window-s.lib"}

filter {}
----------------------------------------------------

project "server"
    kind "ConsoleApp"
    language "C++"
    architecture "x64"
    cppdialect "c++latest"
    staticruntime "On"
    targetdir "bin/%{cfg.buildcfg}"

files {"src/server.cpp"}
includedirs {"include"}

defines {"SFML_STATIC"}

-- sfml deps {in windows}
links {"opengl32.lib"}
links {"winmm.lib"}
links {"ws2_32.lib"}

-- sfml deps {ext}
links {"lib/flac.lib"}
links {"lib/freetype.lib"}
links {"lib/openal32.lib"}
links {"lib/vorbis.lib"}
links {"lib/vorbisenc.lib"}
links {"lib/vorbisfile.lib"}

filter "configurations:Debug"
    runtime "Debug"
    defines {"DEBUG"}
    symbols "On"
    links {"lib/Debug/sfml-audio-s-d.lib"}
    links {"lib/Debug/sfml-graphics-s-d.lib"}
    links {"lib/Debug/sfml-network-s-d.lib"}
    links {"lib/Debug/sfml-system-s-d.lib"}
    links {"lib/Debug/sfml-window-s-d.lib"}

filter "configurations:Release"
    runtime "Release"
    defines {"NDEBUG"}
    optimize "On"
    links {"lib/Release/sfml-audio-s.lib"}
    links {"lib/Release/sfml-graphics-s.lib"}
    links {"lib/Release/sfml-network-s.lib"}
    links {"lib/Release/sfml-system-s.lib"}
    links {"lib/Release/sfml-window-s.lib"}

filter {}
----------------------------------------------------
