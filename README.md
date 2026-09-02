# SFSE Menu Framework Example

A minimal external C++ consumer for SFSE Menu Framework using the header-only
SFSE-MCP SDK. During SFSE
`kPostLoad`, it registers `Test Plugin` > `Settings` > `General`,
`Test Plugin Diagnostics` > `Lifecycle` > `Events`, `Test Plugin Diagnostics`
> `Input and HUD`, `Test Plugin Diagnostics` > `Fonts` > `API`, and a
consumer-owned, resizable ImGui window. They are
available before `kPostDataLoad`. The two roots and nested pages exercise the
framework's search, favorite ordering, archive/restore, and slash-path
navigation. It also registers lifecycle, native-input, and persistent-HUD
callbacks through the public API.

The Fonts page resolves named text fonts by case-insensitive filename and stem
and renders Solid, Regular, and Brands Font Awesome glyphs through the public
push/pop helpers. The multilingual sample also shows that optional glyph
coverage requires both the matching framework setting and a font containing
those characters.

The plugin does not compile or link Dear ImGui. Its callbacks call
`ImGuiMCP::*`; those header-only wrappers resolve the corresponding `ig*`
exports from `SFSEMenuFramework.dll` with `GetProcAddress`, so all ImGui code
executes in the framework DLL that owns the real context.

The General panel exercises the process-lifetime `WindowInterface` returned
by `AddWindow`: the consumer directly controls `IsOpen` and
`BlockUserInput`. It also displays aggregate blocking state, reads the main
framework window, and exercises the framework hotkey control.

The Lifecycle events block counts main-MCP Open/Close events and per-frame
BeforeRender/AfterRender events, shows the most recent event, and reports any
priority-order failure. Its checkbox deletes both RAII `Event` objects to test
unregistration and can register them again. Opening or closing the standalone
window must not change the Open/Close counts.

The Input and HUD page registers a consuming input callback first and an
observing callback second. Arm the next Escape press, close the MCP with F1,
wait for the foreground HUD to update once, then press Escape: Starfield should
remain in the game while the HUD reports matching observed and consumed counts.
Both listener pairs and the HUD can be unregistered and registered again from
the page. The HUD uses only the foreground draw list, remains noninteractive,
and continues to render while the MCP is closed.

An unavailable framework DLL is reported during `kPostLoad` registration and
the example stays inactive.

The hotkey checkbox can be disabled only while the standalone window is open
and blocking. Closing that window or making it nonblocking automatically
restores the hotkey. These safeguards prevent normal use of the example from
stranding the user with F1 disabled.

For a nonblocking-window test, uncheck `Standalone window blocks game input`,
then close the main framework window with F1. The standalone window remains
visible but intentionally has no mouse or keyboard ownership; press F1 again to
return to the General panel.

## Build

```powershell
git submodule update --init
xmake f -m releasedbg
xmake
```

Until SFSE-MCP is consumed as a package, keep its checkout beside this one as
`../SFSE-MCP`; xmake treats it as a header-only dependency.

The output is `build/windows/x64/releasedbg/SFSEMenuFrameworkExample.dll`.

## License

GPL-3.0-only. SFSE-MCP and Dear ImGui remain under their MIT licenses.
