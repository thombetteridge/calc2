add_rules("mode.debug", "mode.release")
add_requires("raylib")
add_requires("fmt")


target("imgui")
    set_kind("static")
    add_files("third_party/imgui/*.cpp")
    add_includedirs("third_party/imgui/", {public = true})
    set_warnings("none")

target("rlimgui")
    set_kind("static")
    add_files("third_party/rlimgui/*.cpp")
    add_includedirs("third_party/imgui")
    add_includedirs("third_party/rlimgui/", {public = true})
    add_packages("raylib")
    set_warnings("none")

target("stack_calc")
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