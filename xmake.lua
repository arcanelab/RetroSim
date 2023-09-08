set_project("RetroSim")
set_version("0.2.0")
set_languages("cxx20")
add_rules("mode.debug", "mode.release")
add_requires("libsdl 2.x")
set_warnings("all")

if is_mode("debug") then
    add_defines("DEBUG")
    set_symbols("debug")
    set_optimize("none")
else 
    set_symbols("hidden")
    set_optimize("fastest")
end

if is_plat("windows") then
    set_warnings("all")
    add_defines("WIN32")
    add_cxflags("/wd4068")
    -- add_ldflags("-subsystem:windows")
elseif is_plat("linux") or is_plat("macosx") or is_plat("mingw") then
    add_defines("UNIX_HOST")
    -- add_cxflags("-Wall")
end

includes("gravity")

Target =
{
    binary = 1,
    libretro = 2
}

local _target = Target.libretro

if _target == Target.binary then
    target("RetroSim")
        add_deps("Gravity")
        set_kind("binary")
        add_files("src/**.cpp")
        add_includedirs("gravity/src/compiler", "gravity/src/optionals", "gravity/src/runtime", "gravity/src/shared", "gravity/src/utils")
        add_packages("libsdl")
        set_targetdir("bin")
elseif _target == Target.libretro then
    target("RetroSimCore")
        add_defines("LIBRETRO")
        add_deps("Gravity")
        set_kind("shared")
        add_files("src/**.cpp")
        add_files("gravity/src/**.c")
        add_includedirs("gravity/src/compiler", "gravity/src/optionals", "gravity/src/runtime", "gravity/src/shared", "gravity/src/utils")
        add_packages("libsdl")
        if os.getenv("RETROARCH_COREPATH") then
            set_targetdir(os.getenv("RETROARCH_COREPATH"))
        else 
            set_targetdir("libretro")
        end
end

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

