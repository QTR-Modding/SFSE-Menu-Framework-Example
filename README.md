# SFSE Menu Framework Example

A C++ example of using the [SFSE-MCP SDK](https://github.com/QTR-Modding/SFSE-MCP)
to add settings pages, a separate window and a HUD to Starfield.
It uses the framework's ImGui implementation, not its own copy.

## Try it

Install [SFSE Menu Framework](https://github.com/QTR-Modding/SFSE-Menu-Framework)
and place `SFSEMenuFrameworkExample.dll` in `Data/SFSE/Plugins`.
Launch through SFSE and press **F1**.

Open **Test Plugin > Settings > General** to try a button and a resizable
window. **Test Plugin Diagnostics** contains:

- **Lifecycle > Events:** menu events and callback priority.
- **Input and HUD:** input consumption and a HUD that stays visible when the
  panel is closed.
- **Fonts > API:** named fonts, Font Awesome icons and optional language glyphs.
- **Menu mutations:** nested paths, replacement, rename and deletion.

Pages register at `kPostLoad`, before game data finishes loading.
If the framework is missing, the example logs a warning and stays inactive.
The SDK's verification rules still apply to a detected invalid framework.

## A few useful checks

- **Nonblocking window:** uncheck `Standalone window blocks game input`,
  then close the main panel with F1. The separate window stays visible while
  input goes to Starfield. Press F1 to return.
- **Hotkey control:** the example only lets you disable the hotkey while its
  separate window is open and blocking. Closing it or making it nonblocking
  restores the hotkey.
- **Input consumption:** arm the next Escape press, close the panel with F1,
  wait for one HUD update, then press Escape. The game should not pause, and
  the HUD should show matching observed and consumed counts.
- **Lifecycle events:** opening the separate window should not change the
  main panel's Open/Close counts. The checkbox removes or restores the listeners.
- **Menu mutations:** visit the slash-named page, then return to the controller.
  The replacement count should rise; the stale-renderer count should stay zero.

For multilingual text, enable the relevant glyph range in the framework and
choose a font that contains those characters.

## Build

Requires Xmake 3.0.9+, MSVC with C++23 support, and the Windows SDK.
Clone the SDK beside this repository at the revision pinned in
[xmake.lua](xmake.lua); the build checks the revision and rejects edited headers.

```powershell
git clone https://github.com/QTR-Modding/SFSE-MCP.git
git -C SFSE-MCP checkout ed331dab06b3055d2d6731a471bccd3587048a17
git clone --recurse-submodules https://github.com/QTR-Modding/SFSE-Menu-Framework-Example.git
cd SFSE-Menu-Framework-Example
xmake f -m releasedbg
xmake
```

The DLL is written to
`build/windows/x64/releasedbg/SFSEMenuFrameworkExample.dll`.
Building does not install it into the game.

## License

[GPL-3.0-only](LICENSE). The SFSE-MCP dependency is MIT-licensed.
