# CS2 Custom Skin Creator

Create your own custom weapon skins by replacing existing skin textures with your own images.

## How It Works

1. You pick a **"carrier" skin** — a cheap/unused existing skin (e.g., Groundwater, Arctic Camo)
2. The tool replaces that skin's texture file with your custom image
3. You apply the carrier's paint kit ID in the cheat menu
4. The game loads **your image** instead of the original skin texture
5. Only you can see it (client-side only)

## Prerequisites

### 1. Python + Pillow
```bash
pip install Pillow
```

### 2. CS2 Workshop Tools DLC (Free)
This provides the `resourcecompiler.exe` needed to compile textures.

**Install it:**
1. Open Steam → Library → Right-click **Counter-Strike 2**
2. Click **Properties** → **DLC**
3. Check ✅ **Counter-Strike 2 Workshop Tools**
4. Wait for download to complete

## Quick Start

### Step 1: Prepare Your Image
- Use **PNG, JPG, or TGA** format
- Best results with **square images** (e.g., 2048x2048)
- The tool auto-resizes to power-of-2 dimensions

### Step 2: Pick a Carrier Skin
See which skins you can override:
```bash
python create_custom_skin.py --list-carriers
```

**Available carriers:**

| Name           | Paint Kit ID | Style        | Best For                       |
|----------------|-------------|--------------|--------------------------------|
| `groundwater`  | 2           | hydrographic | Patterns, abstract art         |
| `arctic_camo`  | 6           | hydrographic | Camo patterns, textures        |
| `forest_ddpat` | 5           | hydrographic | Digital patterns               |
| `storm`        | 25          | hydrographic | Abstract art                   |
| `tornado`      | 17          | hydrographic | Swirl/spiral patterns          |
| `spray`        | 1           | spray_paint  | Stencil-style art              |
| `candy_apple`  | 159         | spray_paint  | Solid base + stencil overlay   |
| `contractor`   | 28          | hydrographic | General purpose                |
| `anodized_navy`| 3           | anodized     | Metallic base                  |

> **TIP:** "hydrographic" carriers tile your image across the weapon surface.
> For best results, use **seamless/tileable images** with these carriers.

### Step 3: Create the Skin
```bash
python create_custom_skin.py --image my_art.png --carrier groundwater
```

With custom resolution:
```bash
python create_custom_skin.py --image my_art.png --carrier arctic_camo --size 1024
```

If CS2 is installed in a non-standard location:
```bash
python create_custom_skin.py --image my_art.png --carrier groundwater --cs2-dir "D:\Steam\steamapps\common\Counter-Strike Global Offensive"
```

### Step 4: Apply In-Game
1. Launch CS2 and inject the cheat
2. Press **INSERT** to open the menu
3. Go to the **Skins** tab
4. Select the weapon you want the skin on
5. **Enable** the skin
6. Set **Custom Paint Kit ID** to the carrier's ID (shown by the tool)
7. Click **Force Update**
8. Your custom image appears on the weapon! 🎨

## Examples

### Apply a Dragon Image on AK-47
```bash
python create_custom_skin.py --image dragon.png --carrier groundwater
```
Then in-game: Skins → AK-47 → Enable → Paint Kit ID: **2** → Force Update

### Custom Camo Pattern on AWP
```bash
python create_custom_skin.py --image custom_camo.png --carrier arctic_camo
```
Then in-game: Skins → AWP → Enable → Paint Kit ID: **6** → Force Update

### Restore Original Skin
If you want to undo a carrier override:
```bash
python create_custom_skin.py --restore groundwater
```

## Creating Great Custom Skins

### Image Tips
- **Resolution**: 2048x2048 gives the sharpest results
- **Seamless tiles**: For hydrographic carriers, seamless images look best
- **High contrast**: Bold colors and high contrast look better on weapons
- **Alpha channel**: Use PNG with transparency for interesting effects

### Style Guide
| Style | How It Maps | Best Image Type |
|-------|-------------|-----------------|
| Hydrographic | Tiles/wraps across the entire weapon surface | Seamless patterns, abstract art, camo |
| Spray Paint | Applied as a stencil on top of the weapon's base | Logos, symbols, stencil art |
| Anodized | Metallic tint with your texture | Subtle textures, gradients |

## Troubleshooting

### "resourcecompiler.exe not found"
Install CS2 Workshop Tools DLC (see Prerequisites above).

### "Could not find CS2 installation directory"
Use the `--cs2-dir` flag to specify your CS2 installation path.

### Compilation fails
1. Make sure Workshop Tools DLC is installed
2. Try running the resource compiler manually:
   ```bash
   "C:\...\Counter-Strike Global Offensive\game\bin\win64\resourcecompiler.exe" -i "path\to\your.vtex"
   ```
3. Check that your image is a valid PNG/JPG/TGA

### Skin doesn't show in-game
1. Make sure you set the **correct Paint Kit ID** for the carrier
2. Click **Force Update** after setting the ID
3. The texture override only works if the compiled `.vtex_c` is in the right directory
4. Verify by checking: `game\csgo\materials\models\weapons\customization\paints\gunsmith\` for the compiled file

### sv_pure kicks me
The override may not work on Valve official servers with `sv_pure 1`.
It works on: community servers (sv_pure 0), offline/LAN, bot matches.

## File Structure
```
Counter-Strike Global Offensive/
├── content/csgo/                          ← Source files go here
│   └── materials/models/weapons/customization/paints/gunsmith/
│       ├── hy_granite.tga                 ← Your custom image (prepared)
│       └── hy_granite.vtex                ← Texture descriptor
│
└── game/csgo/                             ← Compiled files end up here
    └── materials/models/weapons/customization/paints/gunsmith/
        └── hy_granite.vtex_c              ← Compiled texture (game reads this)
```

## Limitations
- **Client-side only** — other players cannot see your custom skin
- **sv_pure** — may not work on official Valve matchmaking servers
- **One skin per carrier** — each carrier can only hold one custom texture at a time
- **UV mapping** — the image maps based on the weapon's UV layout, results vary by weapon
