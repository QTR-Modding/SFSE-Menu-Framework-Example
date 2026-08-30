# SFSE Menu Framework Example

A minimal external C++ consumer for SFSE Menu Framework. It registers
`Test Plugin` > `Settings` during SFSE `kPostLoad` and renders a session-local
counter with an Increment button.

The plugin compiles only the four Dear ImGui 1.90.8 core sources required to
render through the framework-owned context. It does not create a renderer,
platform backend, window hook, or input hook.

## Build

```powershell
git submodule update --init --recursive
xmake f -m releasedbg
xmake
```

The output is `build/windows/x64/releasedbg/SFSEMenuFrameworkExample.dll`.

## License

GPL-3.0-only. Dear ImGui remains under its MIT license.
