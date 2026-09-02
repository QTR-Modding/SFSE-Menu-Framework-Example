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
local plugin_version = "0.6.0"
local plugin_author = "Quantumyilmaz"

set_project(plugin_name)
set_version(plugin_version)
set_license("GPL-3.0-only")
set_languages("c++23")
set_warnings("allextra")
set_encodings("utf-8")

add_rules("mode.debug", "mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

target("sfse-mcp", function()
    set_kind("headeronly")
    set_default(false)
    set_license("MIT")

    local sdk_root = path.join(os.projectdir(), "..", "SFSE-MCP")
    add_headerfiles(path.join(sdk_root, "include", "SFSEMCP", "*.hpp"))
    add_includedirs(path.join(sdk_root, "include"), { public = true })
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

    add_deps("sfse-mcp")
    add_defines("_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs(
        "src"
    )
end)
