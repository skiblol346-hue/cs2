# ttapp

## Overview

ttapp is a Windows x64 C++ CS2 client and injector with a DirectX 11 ImGui menu. The solution builds two outputs:

- `ttapp.dll`: the in-game client.
- `ttapp.exe`: the injector used to load the client.

## Features

- Weapon, knife, glove, and agent cosmetic controls.
- Paint kit search and preview.
- Wear, seed, StatTrak, custom names, custom colors, stickers, and keychains.
- Config create, save, load, delete, and reset.
- Misc toggles for quick stop, spectator list, auto accept, and the menu key.

## Usage

Run the injector as Administrator.

Default:

```powershell
.\bin\x64\Release\ttapp.exe
```

With explicit options:

```powershell
.\bin\x64\Release\ttapp.exe --process cs2.exe --dll .\bin\x64\Release\ttapp.dll --timeout-ms 30000 --logs
```

Defaults:

- `--process` defaults to `cs2.exe`.
- `--dll` defaults to `ttapp.dll` beside the injector.
- The injector waits forever unless `--timeout-ms` is supplied.
- The menu key defaults to Delete.
- Config files are stored in `C:\ttapp\configs\`.

## Build

Requirements:

- Windows.
- Visual Studio 2022 or MSBuild.
- MSVC C++ toolset matching the project setting: `v145`.
- Windows SDK 10.0.

Build Release x64 from the repository root:

```powershell
msbuild ttapp.sln /m /p:Configuration=Release /p:Platform=x64
```

Outputs:

```text
bin\x64\Release\ttapp.exe
bin\x64\Release\ttapp.dll
```

Third-party dependencies are vendored under `vendor/`.

## License

This project uses the BSD Zero Clause License (`0BSD`). It is intentionally permissive: use, copy, modify, redistribute, or sell the software with minimal restriction. GPL is not used because it adds redistribution and source-sharing obligations.

Copyright (C) 2026 ttapp contributors

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
