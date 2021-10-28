add_rules("mode.debug", "mode.release")

target("rangespp")
    set_kind("phony")
    -- add_headerfiles("$(projectdir)/include/**.h", {prefixdir = "include"})
    add_includedirs("$(projectdir)/include", {public = true})

includes("app")