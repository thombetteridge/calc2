---@diagnostic disable: undefined-global

add_rules("mode.debug", "mode.release")
add_requires("raylib")
add_requires("fmt")


if is_mode("debug") then
    if is_mode("debug") and is_plat("linux") then
        add_defines("_GLIBCXX_ASSERTIONS")
        set_policy("build.sanitizer.undefined", true)
    end
    set_policy("build.sanitizer.address", true)       
    set_symbols("debug")
    set_optimize("none")
end


target("imgui",
    {
        kind = "static",
        files = "third_party/imgui/*.cpp",
        includedirs = { "third_party/imgui/", { public = true } },
        warnings = "none",
    })

target("rlimgui", 
    {
        kind = "static",
        files = "third_party/rlimgui/*.cpp",
        includedirs = { "third_party/imgui", "third_party/rlimgui/", { public = true } },
        packages = "raylib",
        warnings = "none",
    })

target("stack_calc")


    if is_mode("debug") and is_plat("linux") then
        add_defines("_GLIBCXX_ASSERTIONS")
    end

    set_languages("c++20")
    set_warnings("all", "extra", "pedantic")
    set_kind("binary")

    set_pcxxheader("source/pch/pch_stdc++.hpp")

    add_files("source/*.cpp")
    add_sysincludedirs("include")

    add_packages("raylib")
    add_packages("fmt")
    add_deps("imgui")
    add_deps("rlimgui")