---@diagnostic disable: undefined-global


add_rules("mode.debug", "mode.release")
set_languages("c++23")
set_warnings("all", "extra")

target("stack_calc")
    set_kind("binary")

    set_pcxxheader("src/pch.hpp")

    add_files("src/*.cpp")
    add_sysincludedirs("ext")

