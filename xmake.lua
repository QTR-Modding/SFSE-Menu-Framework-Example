set_xmakever("3.0.9")
set_policy("package.requires_lock", true)

local project_root = os.projectdir()

if is_plat("windows") then
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

option("sf1sdk_dir")
    set_default("../SF1SDK")
    set_showmenu(true)
    set_description("Directory containing the SF1SDK checkout")
option_end()

includes(get_config("sf1sdk_dir") or "../SF1SDK")
add_requires("spdlog v1.16.0", { configs = { header_only = false, wchar = true, std_format = true } })

local plugin_name = "SFSE Menu Framework Example"
local dll_name = "SFSEMenuFrameworkExample"
local plugin_version = "1.0.0"
local plugin_author = "Quantumyilmaz"
local major, minor, patch = plugin_version:match("^(%d+)%.(%d+)%.(%d+)$")
local packed = tonumber(major) * 16777216 + tonumber(minor) * 65536 + tonumber(patch) * 16
local build_staging_dir = path.join(project_root, "build", "staging")
local sdk_root = path.join(project_root, "..", "SFSE-MCP")
local sdk_revision = "abeee13a366d2a288cb682037c6ec5d577bba810"

local function sdk_checkout_error(run_command)
    local revision = run_command(
        "git",
        { "-C", sdk_root, "rev-parse", "HEAD" }
    ):gsub("%s+$", "")
    if revision ~= sdk_revision then
        return "SFSE-MCP must be checked out at " .. sdk_revision
    end

    local include_changes = run_command(
        "git",
        {
            "-C",
            sdk_root,
            "status",
            "--porcelain",
            "--untracked-files=all",
            "--",
            "include"
        }
    ):gsub("%s+$", "")
    if include_changes ~= "" then
        return "SFSE-MCP include tree must be clean at " .. sdk_revision
    end
end

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

    add_headerfiles(path.join(sdk_root, "include", "SFSEMCP", "**.hpp"))
    add_includedirs(path.join(sdk_root, "include"),
        path.join(sdk_root, "lib/clib-utils-qtr/include"), { public = true })

    on_config(function()
        local error_message = sdk_checkout_error(os.iorunv)
        assert(not error_message, error_message)
    end)
end)

target(dll_name, function()
    set_kind("shared")
    add_rules("sf1sdk.plugin")
    add_deps("SF1SDK")
    add_packages("spdlog")
    add_syslinks("user32")
    add_files("src/Version.rc")
    add_defines(
        "NOMINMAX",
        'PLUGIN_NAME="' .. plugin_name .. '"',
        'PLUGIN_DLL_NAME="' .. dll_name .. '"',
        'PLUGIN_AUTHOR="' .. plugin_author .. '"',
        'PLUGIN_VERSION_TEXT="' .. plugin_version .. '.0"',
        "PLUGIN_VERSION_MAJOR=" .. major,
        "PLUGIN_VERSION_MINOR=" .. minor,
        "PLUGIN_VERSION_PATCH=" .. patch,
        "PLUGIN_PACKED_VERSION=" .. packed
    )

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

    on_config(function(target)
        target:set("installdir", build_staging_dir)
    end)

    before_build(function(target)
        local error_message = sdk_checkout_error(os.iorunv)
        assert(not error_message, error_message)
        assert(
            path.absolute(target:installdir()) == path.absolute(build_staging_dir),
            "refusing to build with a non-staging install destination"
        )
    end)
end)
