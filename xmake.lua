set_project("RetroSim")
set_version("0.2.0")
set_languages("cxx20")
add_rules("mode.debug", "mode.release")
add_requires("libsdl 2.x")
set_warnings("all")

-- libraries built from submodules
includes("src/extern/gravity")
includes("src/cpu/A65000/AsmA65k")
includes("src/extern/csid-mod")

-- make sure to include debug symbols when appropriate
if is_mode("debug") then
    add_defines("DEBUG")
    set_symbols("debug")
    set_optimize("none")
else 
    set_symbols("hidden")
    -- optionally we can include debug symbols in release builds, useful for profiling
    -- set_symbols("debug")
    set_optimize("fastest")
end

-- set platform-specific compiler settings
if is_plat("windows") then
    add_requires("raylib x.x")
    set_warnings("all")
    add_defines("WIN32")
    -- add_cxflags("/wd4068")
    -- add_ldflags("-subsystem:windows")
    -- add_ldflags("-subsystem:console")
elseif is_plat("linux") or is_plat("macosx") or is_plat("mingw") then
    add_defines("UNIX_HOST")
    add_defines("SDL_GPU_DISABLE_GLES")
    -- add_cxflags("-Wall")
end

function AddTelnetDependencies() 
    add_defines("TELNET_ENABLED")
    add_files("src/network/*.cpp")
    add_files("src/extern/libtelnet/*.c")
    add_includedirs("src/extern/libtelnet")
    add_includedirs("src/network")
end

function AddCommon()
    AddTelnetDependencies()
    add_deps("Gravity")
    add_deps("AsmA65k-lib")
    add_deps("csid-mod")
    add_files("src/utils/*.cpp")
    add_files("src/scripting/gravity/*.cpp")
    add_files("src/core/*.cpp")
    add_files("src/cpu/A65000/*.cpp")
    add_files("src/compiled-data/*.cpp")
    add_includedirs("src/compiled-data/")
    add_includedirs("src/scripting/gravity")
    add_includedirs("src/core")
    add_includedirs("src/cpu/A65000")
    add_includedirs("src/cpu/A65000/asmA65k/src")
    add_includedirs("src/utils")
    add_includedirs("src/extern/gravity/src/compiler", "src/extern/gravity/src/optionals", "src/extern/gravity/src/runtime", "src/extern/gravity/src/shared", "src/extern/gravity/src/utils")    
    add_includedirs("src/extern/csid-mod/include")
    set_targetdir("bin")
end

function AddSDL_GPU()
    if is_plat("windows") then
        if is_mode("debug") then
            add_linkdirs("src/extern/sdl-gpu/build/SDL_gpu-VS/lib/Debug")
            -- add_linkdirs("src/extern/SDL/build/Debug")
        end
        if is_mode("release") then
            add_linkdirs("src/extern/sdl-gpu/build/SDL_gpu-VS/lib/Release")
            -- add_linkdirs("src/extern/SDL/build/Release")
        end
        -- add_includedirs("src/extern/sdl/include")
        add_links("sdl2_gpu_s")
        add_links("opengl32")
        add_links("msvcrt")
        -- add_links("sdl2-staticd")
        -- add_links("GL")
    elseif is_plat("macosx") then
        add_deps("BuildSDL_gpu")  
        add_linkdirs("src/extern/sdl-gpu/build/SDL_gpu/lib/")
        add_frameworks("OpenGL")
        add_links("sdl2_gpu")
    end

    add_files("src/system/sdlgpu/*.cpp")
    add_files("src/system/sdlgpu/main.cpp")
    add_includedirs("src/extern/sdl-gpu/include")
end

-- We need to know which SDL target we're building for.
Target =
{
    sdl = 1,
    sdlgpu = 2,
}

local _sdltarget = Target.sdlgpu

target("RetroSim")
    set_default(true)
    AddCommon()
    set_kind("binary")
    add_packages("libsdl")
    if is_plat("windows") then
        add_ldflags("/NODEFAULTLIB:MSVCRT")
    end
    if _sdltarget == Target.sdl then
        add_defines("SDL")
        add_files("src/system/sdl/*.cpp")
    end
    if _sdltarget == Target.sdlgpu then
        AddSDL_GPU()
    end

target("RetroSimCore")
    AddCommon()
    add_defines("LIBRETRO")
    set_kind("shared")
    add_includedirs("src/extern/libretro")
    add_files("src/system/libretro/*.cpp")
    if os.getenv("RETROARCH_COREPATH") then
        set_targetdir(os.getenv("RETROARCH_COREPATH"))
    else 
        set_targetdir("bin")
    end

target("BuildSDL_gpu")
    set_default(false) -- This ensures it's not built with the default `xmake` command.
    if not is_plat("windows") then
        on_build(function()
            if not os.exists("src/extern/sdl-gpu/SDL_gpu/lib/libSDL_gpu.a") then
                print("Invoking cmake for src/extern/SDL-gpu")
                os.exec("sh compile_sdlgpu.sh")
            else
                print("SDL_gpu already compiled.")
            end
        end)
    else
        -- Implement cmake build for Windows
    end

target("Raylib")
    set_default(false)
    AddCommon()
    add_defines("RAYLIB")
    add_files("src/system/raylib/*.cpp")
    add_packages("raylib")
    set_targetdir("bin")
