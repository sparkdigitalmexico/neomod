#!/usr/bin/env python3
"""Probe a pixel in a PNG by normalized coordinates and compare against an expected color.

usage: pixelprobe.py <file.png> <x_frac> <y_frac> <rrggbb> <tolerance>
  x_frac/y_frac are 0..1 fractions of image size, so screenshot scale variance
  (e.g. HiDPI 2560x1440 vs 1280x720 captures) cannot shift the probe point.
prints "PROBE OK ..." or "PROBE FAIL ..." and exits nonzero on failure.
stdlib only (zlib/struct); supports 8-bit RGB/RGBA non-interlaced PNGs.
"""

import struct
import sys
import zlib


def read_png(path):
    with open(path, "rb") as f:
        data = f.read()
    assert data[:8] == b"\x89PNG\r\n\x1a\n", "not a png"
    pos = 8
    width = height = None
    bitdepth = colortype = None
    idat = b""
    while pos < len(data):
        (length,) = struct.unpack(">I", data[pos : pos + 4])
        ctype = data[pos + 4 : pos + 8]
        chunk = data[pos + 8 : pos + 8 + length]
        pos += 12 + length
        if ctype == b"IHDR":
            width, height, bitdepth, colortype, _, _, interlace = struct.unpack(">IIBBBBB", chunk)
            assert bitdepth == 8, f"unsupported bit depth {bitdepth}"
            assert colortype in (2, 6), f"unsupported color type {colortype}"
            assert interlace == 0, "interlaced pngs unsupported"
        elif ctype == b"IDAT":
            idat += chunk
        elif ctype == b"IEND":
            break
    raw = zlib.decompress(idat)
    channels = 3 if colortype == 2 else 4
    stride = width * channels
    out = bytearray(height * stride)
    prev = bytearray(stride)
    pos = 0
    for y in range(height):
        filt = raw[pos]
        line = bytearray(raw[pos + 1 : pos + 1 + stride])
        pos += 1 + stride
        if filt == 1:  # sub
            for i in range(channels, stride):
                line[i] = (line[i] + line[i - channels]) & 0xFF
        elif filt == 2:  # up
            for i in range(stride):
                line[i] = (line[i] + prev[i]) & 0xFF
        elif filt == 3:  # average
            for i in range(stride):
                a = line[i - channels] if i >= channels else 0
                line[i] = (line[i] + ((a + prev[i]) >> 1)) & 0xFF
        elif filt == 4:  # paeth
            for i in range(stride):
                a = line[i - channels] if i >= channels else 0
                b = prev[i]
                c = prev[i - channels] if i >= channels else 0
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pred) & 0xFF
        out[y * stride : (y + 1) * stride] = line
        prev = line
    return width, height, channels, out


def probe(path, xf, yf, expected, tol):
    """compare the pixel at normalized (xf, yf) against #expected within tolerance.
    returns (ok, message) where message is the PROBE OK/FAIL report line."""
    width, height, channels, pixels = read_png(path)
    x = min(width - 1, max(0, int(xf * width)))
    y = min(height - 1, max(0, int(yf * height)))
    off = (y * width + x) * channels
    r, g, b = pixels[off], pixels[off + 1], pixels[off + 2]
    er, eg, eb = (int(expected[i : i + 2], 16) for i in (0, 2, 4))
    ok = abs(r - er) <= tol and abs(g - eg) <= tol and abs(b - eb) <= tol
    verdict = "OK" if ok else "FAIL"
    message = (
        f"PROBE {verdict} {path} ({xf},{yf})->({x},{y}) "
        f"expected=#{expected} actual=#{r:02x}{g:02x}{b:02x} tol={tol} [{width}x{height}]"
    )
    return ok, message


def main():
    if len(sys.argv) != 6:
        print(__doc__.strip())
        return 2
    ok, message = probe(
        sys.argv[1], float(sys.argv[2]), float(sys.argv[3]), sys.argv[4], int(sys.argv[5])
    )
    print(message)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
