#!/usr/bin/env python3
"""Set the ProDOS aux type of a file in a .po / .hdv image.

    python3 setaux.py IMAGE.po FOLDER/NAME 0x5257

AppleCommander only sets the aux type of BIN-style files; RetroWrite II
documents are type $50 (GWP) with aux type $5257, so this patches the
directory entry directly.
"""
import sys


def blk(img, n):
    return img[n * 512:(n + 1) * 512]


def entries(img, key):
    """yield (block, offset, entry bytes) for each active entry in a directory"""
    b = key
    first = True
    while b:
        d = blk(img, b)
        for k in range(13):
            if first and k == 0:
                continue
            off = 4 + k * 0x27
            e = d[off:off + 0x27]
            if e[0] & 0xF0:
                yield b, off, e
        first = False
        b = d[2] | d[3] << 8


def find(img, path):
    key = 2
    parts = [p for p in path.upper().split('/') if p]
    for i, name in enumerate(parts):
        for b, off, e in entries(img, key):
            n = e[0] & 0x0F
            if e[1:1 + n].decode('ascii') == name:
                if i == len(parts) - 1:
                    return b * 512 + off
                key = e[0x11] | e[0x12] << 8
                break
        else:
            raise SystemExit(f'{path}: not found')


def setaux(image, path, aux):
    img = bytearray(open(image, 'rb').read())
    pos = find(img, path)
    img[pos + 0x1F] = aux & 0xFF
    img[pos + 0x20] = aux >> 8
    open(image, 'wb').write(img)


if __name__ == '__main__':
    setaux(sys.argv[1], sys.argv[2], int(sys.argv[3], 0))
