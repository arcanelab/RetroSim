set_targetdir("lib")

if is_plat("windows") then
    add_defines("WIN32")
elseif is_plat("linux") or is_plat("macosx") or is_plat("mingw") then
    add_defines("UNIX_HOST")
end

target("picoc")
    set_kind("static")
    add_files("**.c")
