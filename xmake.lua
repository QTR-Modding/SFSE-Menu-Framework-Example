set_xmakever("3.0.9")
set_policy("package.requires_lock", true)

if is_plat("windows") then
    local project_root = os.projectdir()
    add_cxflags(
        "/Brepro",
        "/experimental:deterministic",
        '/d1trimfile:"' .. project_root .. '"',
        '/pathmap:"' .. project_root .. '"=.',
        {
            tools = "cl",
            force = true
        })
    add_shflags("/Brepro", "/PDBALTPATH:%_PDB%", {
        force = true
    })
end

includes(path.join(os.projectdir(), "lib", "commonlibsf"))

local plugin_name = "SFSE Menu Framework Example"
local dll_name = "SFSEMenuFrameworkExample"
local plugin_version = "0.4.0"
local plugin_author = "Quantumyilmaz"

set_project(plugin_name)
set_version(plugin_version)
set_license("GPL-3.0-only")
set_languages("c++23")
set_warnings("allextra")
set_encodings("utf-8")

add_rules("mode.debug", "mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

target("imgui-core", function()
    set_kind("static")
    set_default(false)
    set_license("MIT")

    add_files(
        "extern/imgui/imgui.cpp",
        "extern/imgui/imgui_draw.cpp",
        "extern/imgui/imgui_tables.cpp",
        "extern/imgui/imgui_widgets.cpp"
    )
    add_headerfiles(
        "extern/imgui/imconfig.h",
        "extern/imgui/imgui.h",
        "extern/imgui/imgui_internal.h",
        "extern/imgui/imstb_rectpack.h",
        "extern/imgui/imstb_textedit.h",
        "extern/imgui/imstb_truetype.h"
    )
    add_includedirs("extern/imgui", { public = true })
end)

target(dll_name, function()
    add_rules("commonlibsf.plugin", {
        author = plugin_author,
        name = plugin_name,
        description = plugin_name,
        options = {
            sig_scanning = false,
            address_library = false,
            no_struct_use = true,
            layout_dependent = false
        }
    })

    set_version(plugin_version)
    set_license("GPL-3.0-only")
    set_pcxxheader("src/PCH.h")

    add_deps("imgui-core")
    add_defines("_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs(
        "src",
        "lib/sfse-menu-framework/include"
    )
end)
