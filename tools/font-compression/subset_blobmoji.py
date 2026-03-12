#!/usr/bin/env python3
"""
Subset blobmoji.ttf: remove skin tone variant bitmaps but keep the modifier
codepoints mapped to a zero-width empty glyph, so skin-toned emoji sequences
gracefully degrade to their neutral/yellow base.

Then re-compress all embedded PNGs with oxipng for better compression.
"""

from fontTools.ttLib import TTFont
from fontTools.subset import Subsetter, Options
import os
import subprocess
import tempfile

SKIN_TONE_MODIFIERS = set(range(0x1F3FB, 0x1F400))  # U+1F3FB..U+1F3FF

src = "blobmoji.ttf"
dst = "blobmoji.woff2"

print(f"Loading {src}...")
font = TTFont(src)

# --- Step 1: Subset to remove skin tone variant bitmaps ---
cmap = font.getBestCmap()
all_codepoints = set(cmap.keys())
keep_codepoints = all_codepoints - SKIN_TONE_MODIFIERS

opts = Options()
opts.layout_features = ["*"]

subsetter = Subsetter(options=opts)
subsetter.populate(unicodes=keep_codepoints)
subsetter.subset(font)

# --- Step 2: Add skin tone modifiers back as zero-width glyphs ---
empty_glyph_name = "empty.skintone"
glyph_order = font.getGlyphOrder()
if empty_glyph_name not in glyph_order:
    font.setGlyphOrder(glyph_order + [empty_glyph_name])

font["hmtx"].metrics[empty_glyph_name] = (0, 0)

glyf_table = font.get("glyf")
if glyf_table is not None:
    from fontTools.ttLib.tables._g_l_y_f import Glyph
    glyf_table.glyphs[empty_glyph_name] = Glyph()

if "vmtx" in font:
    font["vmtx"].metrics[empty_glyph_name] = (0, 0)

for table in font["cmap"].tables:
    if hasattr(table, "cmap"):
        for cp in SKIN_TONE_MODIFIERS:
            table.cmap[cp] = empty_glyph_name

# --- Step 3: Re-compress embedded PNGs with oxipng ---
if "CBDT" in font:
    cbdt = font["CBDT"]
    png_count = 0
    saved_bytes = 0

    PNG_SIG = b"\x89PNG"

    with tempfile.TemporaryDirectory() as tmpdir:
        # Collect all bitmap data references
        # Format 17: 9 bytes of metrics header, then PNG data
        entries = []
        for strike_data in cbdt.strikeData:
            for glyph_name, bitmap in strike_data.items():
                if not hasattr(bitmap, "data"):
                    continue
                png_offset = bitmap.data.find(PNG_SIG)
                if png_offset >= 0:
                    entries.append((glyph_name, bitmap, png_offset))

        if entries:
            print(f"Optimizing {len(entries)} embedded PNGs with oxipng...")

            # Write PNG portions to temp files
            paths = []
            for glyph_name, bitmap, png_offset in entries:
                path = os.path.join(tmpdir, f"{glyph_name}.png")
                with open(path, "wb") as f:
                    f.write(bitmap.data[png_offset:])
                paths.append(path)

            # Run oxipng on all files at once
            subprocess.run(
                ["oxipng", "-o", "max", "-s", "--quiet"] + paths,
                check=True,
            )

            # Read back optimized PNGs, reconstruct with header
            for (glyph_name, bitmap, png_offset), path in zip(entries, paths):
                with open(path, "rb") as f:
                    new_png = f.read()
                old_size = len(bitmap.data) - png_offset
                new_size = len(new_png)
                if new_size < old_size:
                    saved_bytes += old_size - new_size
                    png_count += 1
                    # Reconstruct: metrics header + optimized PNG
                    # Update the data length in the header (bytes 5-8, big-endian uint32)
                    header = bytearray(bitmap.data[:png_offset])
                    header[5:9] = len(new_png).to_bytes(4, "big")
                    bitmap.data = bytes(header) + new_png

    print(f"Optimized {png_count} PNGs, saved {saved_bytes / 1024:.1f} KB of bitmap data")

# --- Step 4: Save as WOFF2 ---
font.flavor = "woff2"
print(f"Saving {dst}...")
font.save(dst)

orig_size = os.path.getsize(src)
new_size = os.path.getsize(dst)
print(f"Original: {orig_size / 1024 / 1024:.2f} MB ({src})")
print(f"Subset:   {new_size / 1024 / 1024:.2f} MB ({dst})")
print(f"Savings:  {(orig_size - new_size) / 1024 / 1024:.2f} MB ({100 * (1 - new_size / orig_size):.1f}%)")
