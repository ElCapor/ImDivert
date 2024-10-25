add_requires("raylib", "imgui", {configs = {win32=false}})


package("boostregex")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "ext/ImGuiColorTextEdit/vendor/regex"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DBOOST_REGEX_STANDALONE=ON")
        import("package.tools.cmake").install(package, configs)
    end)
    
package_end()

add_requires("boostregex")
add_requires("luajit", "sol2", {configs = {includes_lua=false}})


target("divertor")
    add_files("src/**.cpp")
    set_symbols("debug")
    set_languages("cxx17")
    set_kind("binary")
    add_linkdirs("ext/windivert/x64")
    add_includedirs("ext/windivert/include", "include", "ext/ImGuiColorTextEdit", "ext/ImGuiColorTextEdit/vendor/regex/include")
    add_files("ext/ImGuiColorTextEdit/TextEditor.cpp", "ext/ImGuiColorTextEdit/LanguageDefinitions.cpp")
    add_links("WinDivert.lib", "ws2_32")
    add_packages("raylib", "imgui", "boostregex", "luajit", "sol2")
