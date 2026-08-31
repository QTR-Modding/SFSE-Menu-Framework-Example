# SFSE Menu Framework Example

A minimal external C++ consumer for SFSE Menu Framework. During SFSE
`kPostLoad`, it registers `Test Plugin` > `Settings` and a consumer-owned,
resizable ImGui window through the public `AddWindow` API. Both are available
before `kPostDataLoad`.

The plugin compiles only the four Dear ImGui 1.90.8 core sources required to
render through the framework-owned context. It does not create a renderer,
platform backend, window hook, or input hook.

The settings panel exercises the process-lifetime `WindowInterface` returned
by `AddWindow`: the consumer directly controls `IsOpen` and
`BlockUserInput`. It also displays aggregate blocking state, reads the main
framework window, and exercises the framework hotkey control.

The hotkey checkbox can be disabled only while the standalone window is open
and blocking. Closing that window or making it nonblocking automatically
restores the hotkey. These safeguards prevent normal use of the example from
stranding the user with F1 disabled.

For a nonblocking-window test, uncheck `Standalone window blocks game input`,
then close the main framework window with F1. The standalone window remains
visible but intentionally has no mouse or keyboard ownership; press F1 again to
return to the settings panel.

## Build

```powershell
git submodule update --init --recursive
xmake f -m releasedbg
xmake
```

The output is `build/windows/x64/releasedbg/SFSEMenuFrameworkExample.dll`.

## License

GPL-3.0-only. Dear ImGui remains under its MIT license.
