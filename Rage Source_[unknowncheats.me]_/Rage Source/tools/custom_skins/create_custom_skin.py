"""
CS2 Custom Skin Creator Tool
=============================
Creates texture overrides so you can replace existing weapon skin textures
with your own custom images (PNG/TGA).

How it works:
  1. You pick a "carrier" paint kit — an existing cheap/unused skin
  2. This tool replaces that skin's texture with your custom image
  3. It patches gameinfo.gi to add a custom search path (loaded BEFORE VPKs)
  4. You apply the carrier's paint kit ID via the skin changer in-game
  5. The game loads YOUR texture instead of the original

Requirements:
  - Python 3.8+
  - Pillow:  pip install Pillow
  - CS2 Workshop Tools DLC (free, install via Steam -> CS2 -> DLC -> Workshop Tools)

Usage:
  python create_custom_skin.py --image my_art.png --carrier groundwater
  python create_custom_skin.py --image my_art.png --carrier arctic_camo --size 2048
  python create_custom_skin.py --list-carriers
  python create_custom_skin.py --restore groundwater

Author: Cs2 Rage Custom Skin Tool
"""

import argparse
import os
import sys
import shutil
import subprocess
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("[!] Pillow is required. Install it: pip install Pillow")
    sys.exit(1)


# The name of our custom override directory (placed alongside csgo/ in game/)
CUSTOM_DIR_NAME = "csgo_custom_skins"


# ============================================================
# Carrier Skins — existing cheap skins whose textures we replace
# ============================================================
CARRIERS = {
    "groundwater": {
        "paint_kit_id": 2,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/hy_granite",
        "style": "hydrographic",
        "notes": "Consumer-grade, tiles across weapon. Good for patterns/textures."
    },
    "arctic_camo": {
        "paint_kit_id": 6,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/hy_arctic",
        "style": "hydrographic",
        "notes": "Mil-Spec, tiles/wraps. Good for camo-style patterns."
    },
    "forest_ddpat": {
        "paint_kit_id": 5,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/hy_forest_ddpat",
        "style": "hydrographic",
        "notes": "Industrial-grade, digital camo pattern. Good for any tiling texture."
    },
    "candy_apple": {
        "paint_kit_id": 159,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/sp_tape_short",
        "style": "spray_paint",
        "notes": "Mil-Spec spray paint -- solid base with your image as the stencil overlay."
    },
    "anodized_navy": {
        "paint_kit_id": 3,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/an_navy",
        "style": "anodized",
        "notes": "Mil-Spec anodized. Simple metallic base."
    },
    "storm": {
        "paint_kit_id": 25,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/hy_storm",
        "style": "hydrographic",
        "notes": "Industrial-grade, good for abstract patterns."
    },
    "tornado": {
        "paint_kit_id": 17,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/hy_tornado",
        "style": "hydrographic",
        "notes": "Industrial-grade, spiral/swirl pattern."
    },
    "spray": {
        "paint_kit_id": 1,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/sp_spray",
        "style": "spray_paint",
        "notes": "Most basic skin, Consumer-grade. Spray paint style."
    },
    "contractor": {
        "paint_kit_id": 28,
        "texture_path": "materials/models/weapons/customization/paints/gunsmith/hy_contractor",
        "style": "hydrographic",
        "notes": "Mil-Spec, good general-purpose carrier."
    },
}


def find_cs2_directory():
    """Try to auto-detect CS2 installation directory."""
    common_paths = [
        r"F:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive",
        r"C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive",
        r"D:\Steam\steamapps\common\Counter-Strike Global Offensive",
        r"D:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive",
        r"E:\Steam\steamapps\common\Counter-Strike Global Offensive",
        r"E:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive",
        r"C:\Steam\steamapps\common\Counter-Strike Global Offensive",
    ]
    for p in common_paths:
        if os.path.isdir(p):
            return p
    return None


def find_resource_compiler(cs2_dir):
    """Find resourcecompiler.exe in CS2 installation."""
    rc_path = os.path.join(cs2_dir, "game", "bin", "win64", "resourcecompiler.exe")
    if os.path.isfile(rc_path):
        return rc_path
    return None


def patch_gameinfo(cs2_dir):
    """
    Patch gameinfo.gi to add our custom search path BEFORE 'Game csgo'.
    
    In Source 2, VPK files within a Game search path take priority over loose files
    in the SAME path. But a search path listed EARLIER takes priority over later ones.
    
    So by adding 'Game csgo_custom_skins' BEFORE 'Game csgo', our loose .vtex_c files
    in csgo_custom_skins/ will be found before the VPK-packed originals in csgo/.
    """
    gameinfo_path = os.path.join(cs2_dir, "game", "csgo", "gameinfo.gi")
    
    if not os.path.isfile(gameinfo_path):
        print(f"[-] gameinfo.gi not found at: {gameinfo_path}")
        return False
    
    with open(gameinfo_path, "r", encoding="utf-8") as f:
        content = f.read()
    
    search_line = f"Game\t{CUSTOM_DIR_NAME}"
    
    # Already patched?
    if CUSTOM_DIR_NAME in content:
        print(f"[+] gameinfo.gi already patched (has {CUSTOM_DIR_NAME})")
        return True
    
    # Backup
    backup_path = gameinfo_path + ".backup"
    if not os.path.isfile(backup_path):
        shutil.copy2(gameinfo_path, backup_path)
        print(f"[+] Backed up gameinfo.gi -> {backup_path}")
    
    # Insert our custom path BEFORE the first 'Game    csgo' line
    # We look for the line that has 'Game' followed by whitespace and 'csgo' (but not csgo_*)
    lines = content.splitlines(keepends=True)
    new_lines = []
    inserted = False
    
    for line in lines:
        stripped = line.strip()
        # Match the first 'Game    csgo' line (not csgo_imported, csgo_core, etc.)
        if not inserted and stripped.startswith("Game") and stripped.endswith("csgo") and "csgo_" not in stripped:
            # Get the indentation from this line
            indent = line[:len(line) - len(line.lstrip())]
            # Insert our custom path before it
            new_lines.append(f"{indent}Game\t{CUSTOM_DIR_NAME}\n")
            print(f"[+] Injected 'Game {CUSTOM_DIR_NAME}' into gameinfo.gi SearchPaths")
            inserted = True
        new_lines.append(line)
    
    if not inserted:
        print("[-] Could not find 'Game csgo' line in gameinfo.gi to patch")
        print("    You may need to manually add this line to SearchPaths:")
        print(f'    Game\t{CUSTOM_DIR_NAME}')
        return False
    
    with open(gameinfo_path, "w", encoding="utf-8") as f:
        f.writelines(new_lines)
    
    print("[+] gameinfo.gi patched successfully!")
    return True


def restore_gameinfo(cs2_dir):
    """Restore original gameinfo.gi from backup."""
    gameinfo_path = os.path.join(cs2_dir, "game", "csgo", "gameinfo.gi")
    backup_path = gameinfo_path + ".backup"
    
    if os.path.isfile(backup_path):
        shutil.copy2(backup_path, gameinfo_path)
        print("[+] Restored original gameinfo.gi")
        return True
    else:
        print("[*] No gameinfo.gi backup found")
        return False


def prepare_image(image_path, output_path, size=2048):
    """Load, resize, and save image as TGA for Source 2 compilation."""
    img = Image.open(image_path)

    # Convert to RGBA
    if img.mode != "RGBA":
        img = img.convert("RGBA")

    # Resize to power-of-two dimensions
    img = img.resize((size, size), Image.Resampling.LANCZOS)

    # Save as TGA (Source 2 preferred format)
    tga_path = os.path.splitext(output_path)[0] + ".tga"
    img.save(tga_path, format="TGA")
    print(f"[+] Prepared image: {tga_path} ({size}x{size})")
    return tga_path


def create_vtex_descriptor(tga_relative_path, output_vtex_path):
    """Create a .vtex descriptor file for Source 2 resource compiler."""
    content = f"""<!-- dmx encoding keyvalues2_noids 1 format vtex 1 -->
"CDmeVtex"
{{
    "m_inputTextureArray" "element_array"
    [
        "CDmeInputTexture"
        {{
            "m_name" "string" "0"
            "m_fileName" "string" "{tga_relative_path}"
            "m_colorSpace" "string" "srgb"
            "m_typeString" "string" "2D"
        }}
    ]
    "m_outputTypeString" "string" "2D"
    "m_outputFormat" "string" "DXT5"
    "m_textureOutputChannelArray" "element_array"
    [
        "CDmeTextureOutputChannel"
        {{
            "m_inputTextureArray" "string_array" [ "0" ]
            "m_srcChannels" "string" "rgba"
            "m_dstChannels" "string" "rgba"
            "m_mipAlgorithm" "CDmeImageProcessor"
            {{
                "m_algorithm" "string" ""
                "m_stringArg" "string" ""
                "m_vFloat4Arg" "vector4" "0 0 0 0"
            }}
            "m_outputColorSpace" "string" "srgb"
        }}
    ]
}}
"""
    with open(output_vtex_path, "w") as f:
        f.write(content)
    print(f"[+] Created .vtex descriptor: {output_vtex_path}")
    return output_vtex_path


def compile_texture(resource_compiler, vtex_path, cs2_dir, output_game_dir):
    """Run Source 2 resource compiler to compile .vtex -> .vtex_c"""
    cmd = [
        resource_compiler,
        "-i", vtex_path,
        "-game", output_game_dir,
    ]
    print(f"[*] Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=cs2_dir)

    if result.returncode == 0:
        print("[+] Compilation successful!")
        if result.stdout:
            print(result.stdout[:500])
    else:
        print(f"[-] Compilation failed (exit code {result.returncode})")
        if result.stderr:
            print(result.stderr[:500])
        if result.stdout:
            print(result.stdout[:500])
    return result.returncode == 0


def create_custom_skin(args):
    """Main workflow: create a custom skin override."""
    carrier_name = args.carrier.lower()
    if carrier_name not in CARRIERS:
        print(f"[-] Unknown carrier '{carrier_name}'. Use --list-carriers to see available carriers.")
        sys.exit(1)

    carrier = CARRIERS[carrier_name]
    image_path = args.image
    size = args.size

    if not os.path.isfile(image_path):
        print(f"[-] Image not found: {image_path}")
        sys.exit(1)

    # Find CS2 directory
    cs2_dir = args.cs2_dir or find_cs2_directory()
    if not cs2_dir or not os.path.isdir(cs2_dir):
        print("[-] Could not find CS2 installation directory.")
        print("    Use --cs2-dir to specify it manually.")
        sys.exit(1)

    print(f"[+] CS2 directory: {cs2_dir}")

    # Find resource compiler
    rc_path = find_resource_compiler(cs2_dir)
    if not rc_path:
        print("[-] resourcecompiler.exe not found!")
        print("    Make sure CS2 Workshop Tools DLC is installed.")
        sys.exit(1)

    print(f"[+] Resource compiler: {rc_path}")
    print(f"[+] Carrier skin: {carrier_name} (Paint Kit ID: {carrier['paint_kit_id']})")
    print(f"[+] Style: {carrier['style']}")
    print(f"[+] Input image: {image_path}")
    print()

    # Set up directory structure
    # We compile INTO our custom override directory, not the main csgo/ folder
    texture_rel_path = carrier["texture_path"]
    content_dir = os.path.join(cs2_dir, "content", "csgo")
    custom_game_dir = os.path.join(cs2_dir, "game", CUSTOM_DIR_NAME)

    # Create the custom game directory
    os.makedirs(custom_game_dir, exist_ok=True)

    # Create content directory for the texture source
    texture_dir = os.path.join(content_dir, os.path.dirname(texture_rel_path))
    os.makedirs(texture_dir, exist_ok=True)

    texture_basename = os.path.basename(texture_rel_path)

    # Step 1: Patch gameinfo.gi to add our custom search path
    print("--- Step 1: Patching gameinfo.gi ---")
    if not patch_gameinfo(cs2_dir):
        print("[!] Warning: gameinfo.gi patch may have failed. Continuing anyway...")
    print()

    # Step 2: Prepare the image (resize to power-of-two, convert to TGA)
    print("--- Step 2: Preparing image ---")
    tga_filename = texture_basename + ".tga"
    tga_output_path = os.path.join(texture_dir, tga_filename)
    prepare_image(image_path, tga_output_path, size)
    print()

    # Step 3: Create .vtex descriptor
    print("--- Step 3: Creating .vtex descriptor ---")
    vtex_filename = texture_basename + ".vtex"
    vtex_output_path = os.path.join(texture_dir, vtex_filename)
    tga_relative_to_content = os.path.relpath(tga_output_path, content_dir).replace("\\", "/")
    create_vtex_descriptor(tga_relative_to_content, vtex_output_path)
    print()

    # Step 4: Compile with resource compiler — output to our custom directory
    print("--- Step 4: Compiling texture ---")
    success = compile_texture(rc_path, vtex_output_path, cs2_dir, custom_game_dir)

    if not success:
        # Fallback: try compiling to csgo/ and then moving the file
        print("[*] Trying fallback: compile to csgo/ then copy...")
        csgo_game_dir = os.path.join(cs2_dir, "game", "csgo")
        success = compile_texture(rc_path, vtex_output_path, cs2_dir, csgo_game_dir)
        if success:
            # Move compiled file from csgo/ to our custom dir
            src = os.path.join(csgo_game_dir, texture_rel_path + ".vtex_c")
            dst_dir = os.path.join(custom_game_dir, os.path.dirname(texture_rel_path))
            os.makedirs(dst_dir, exist_ok=True)
            dst = os.path.join(dst_dir, texture_basename + ".vtex_c")
            if os.path.isfile(src):
                shutil.copy2(src, dst)
                print(f"[+] Copied compiled texture to: {dst}")

    if success:
        # Verify the file exists in our custom directory
        expected_output = os.path.join(custom_game_dir, texture_rel_path + ".vtex_c")
        if not os.path.isfile(expected_output):
            # Check if it went to csgo/ instead
            csgo_output = os.path.join(cs2_dir, "game", "csgo", texture_rel_path + ".vtex_c")
            if os.path.isfile(csgo_output):
                dst_dir = os.path.join(custom_game_dir, os.path.dirname(texture_rel_path))
                os.makedirs(dst_dir, exist_ok=True)
                shutil.copy2(csgo_output, expected_output)
                print(f"[+] Copied to custom directory: {expected_output}")

        print("\n" + "=" * 60)
        print("  CUSTOM SKIN CREATED SUCCESSFULLY!")
        print("=" * 60)
        print(f"\n  Carrier skin:   {carrier_name}")
        print(f"  Paint Kit ID:   {carrier['paint_kit_id']}")
        print(f"  Style:          {carrier['style']}")
        print(f"  Override dir:   game/{CUSTOM_DIR_NAME}/")
        print(f"\n  HOW TO USE IN-GAME:")
        print(f"  1. *** RESTART CS2 *** (required for gameinfo.gi changes)")
        print(f"  2. Open the cheat menu (INSERT)")
        print(f"  3. Go to the Skins tab")
        print(f"  4. Select the weapon you want")
        print(f"  5. Enable the skin and set Paint Kit ID to: {carrier['paint_kit_id']}")
        print(f"  6. Click 'Force Update'")
        print(f"\n  Your custom image will appear on the weapon!")
        print(f"\n  NOTE: Only YOU can see this skin (client-side only).")
        print(f"  NOTE: You MUST restart CS2 after the first run of this tool!")
        print("=" * 60)
    else:
        print("\n[-] Compilation failed.")
        print(f"    Source TGA: {tga_output_path}")
        print(f"    Vtex descriptor: {vtex_output_path}")
        print(f"\n    Manual compilation command:")
        print(f'    "{rc_path}" -i "{vtex_output_path}" -game "{custom_game_dir}"')


def restore_carrier(args):
    """Restore a carrier skin's original texture (remove override)."""
    carrier_name = args.restore.lower()
    if carrier_name not in CARRIERS:
        print(f"[-] Unknown carrier '{carrier_name}'.")
        sys.exit(1)

    cs2_dir = args.cs2_dir or find_cs2_directory()
    if not cs2_dir:
        print("[-] Could not find CS2 directory.")
        sys.exit(1)

    carrier = CARRIERS[carrier_name]
    custom_game_dir = os.path.join(cs2_dir, "game", CUSTOM_DIR_NAME)
    override_path = os.path.join(custom_game_dir, carrier["texture_path"] + ".vtex_c")

    if os.path.isfile(override_path):
        os.remove(override_path)
        print(f"[+] Removed custom override for '{carrier_name}'")
        print(f"    Game will load original from VPK on next restart.")
    else:
        print(f"[*] No override found for '{carrier_name}' — already using original.")

    # Also clean up any leftover in csgo/ directory
    csgo_override = os.path.join(cs2_dir, "game", "csgo", carrier["texture_path"] + ".vtex_c")
    backup = csgo_override + ".backup"
    if os.path.isfile(backup):
        shutil.copy2(backup, csgo_override)
        os.remove(backup)
        print(f"[+] Restored original file in csgo/ from backup")
    elif os.path.isfile(csgo_override):
        # Check if it's our file (recently modified) vs original from VPK extraction
        os.remove(csgo_override)
        print(f"[+] Removed loose file from csgo/")


def list_carriers():
    """Print all available carrier skins."""
    print("\n Available Carrier Skins")
    print("=" * 70)
    print(f"  {'Name':<18} {'ID':<6} {'Style':<16} Notes")
    print("-" * 70)
    for name, info in CARRIERS.items():
        print(f"  {name:<18} {info['paint_kit_id']:<6} {info['style']:<16} {info['notes']}")
    print()
    print("  TIP: 'hydrographic' carriers tile your image across the weapon.")
    print("       'spray_paint' carriers apply your image as a stencil overlay.")
    print("       For full-coverage art, use a hydrographic carrier with a")
    print("       seamless/tileable image for best results.")
    print()


def main():
    parser = argparse.ArgumentParser(
        description="CS2 Custom Skin Creator -- Replace existing skin textures with your own images",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python create_custom_skin.py --image dragon.png --carrier groundwater
  python create_custom_skin.py --image my_camo.png --carrier arctic_camo --size 1024
  python create_custom_skin.py --list-carriers
  python create_custom_skin.py --restore groundwater
        """
    )

    parser.add_argument("--image", "-i", help="Path to your custom image (PNG/JPG/TGA)")
    parser.add_argument("--carrier", "-c", help="Name of the carrier skin to override")
    parser.add_argument("--size", "-s", type=int, default=2048,
                        help="Texture resolution (power of 2). Default: 2048")
    parser.add_argument("--cs2-dir", help="CS2 installation directory (auto-detected if not specified)")
    parser.add_argument("--list-carriers", action="store_true", help="List available carrier skins")
    parser.add_argument("--restore", help="Restore a carrier skin's original texture")

    args = parser.parse_args()

    if args.list_carriers:
        list_carriers()
        return

    if args.restore:
        restore_carrier(args)
        return

    if not args.image or not args.carrier:
        parser.print_help()
        print("\n[!] Both --image and --carrier are required.")
        print("    Use --list-carriers to see available carrier skins.")
        sys.exit(1)

    create_custom_skin(args)


if __name__ == "__main__":
    main()
