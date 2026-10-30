#!/usr/bin/env python3
"""Convert the 6502 Retro Computing graphic into RW2.SPLASH (double hi-res)."""
import sys
from collections import Counter
from PIL import Image
sys.path.insert(0, __import__('os').path.dirname(__file__))
from mksplash import encode

SRC = {  # source colour -> DHGR nibble (stream order, calibrated)
    (0, 0, 0): 0, (255, 255, 255): 15, (152, 201, 151): 7, (243, 187, 68): 14,
    (230, 136, 60): 12, (208, 73, 64): 8, (139, 66, 146): 9, (73, 152, 209): 3,
    (119, 185, 87): 6,
}
PAL = {0: (0, 0, 0), 1: (38, 35, 246), 2: (0, 124, 20), 3: (0, 160, 245), 4: (84, 87, 0),
       5: (121, 122, 122), 6: (36, 212, 0), 7: (74, 243, 143), 8: (170, 0, 103), 9: (207, 33, 245),
       10: (124, 122, 122), 11: (162, 157, 243), 12: (243, 84, 0), 13: (243, 119, 227),
       14: (205, 211, 0), 15: (243, 245, 245)}

def classify(c):
    return min(SRC, key=lambda s: sum((a - b) ** 2 for a, b in zip(s, c)))

def main(src, out, preview):
    im = Image.open(src).convert('RGB')
    W, H = im.size
    # square crop around the content
    x0, y0, x1, y1 = int(W * 0.039), int(H * 0.065), int(W * 0.961), int(H * 0.987)
    im = im.crop((x0, y0, x1, y1))
    lines, top = 152, 4
    px = round(lines / 1.829)              # DHGR pixels are ~1.83x wider than tall
    left = (140 - px) // 2
    small = im.resize((px * 4, lines * 4), Image.NEAREST)
    cache = {}
    nib = [[0] * 140 for _ in range(192)]
    for y in range(lines):
        for x in range(px):
            cnt = Counter()
            for dy in range(4):
                for dx in range(4):
                    c = small.getpixel((x * 4 + dx, y * 4 + dy))
                    if c not in cache:
                        cache[c] = SRC[classify(c)]
                    cnt[cache[c]] += 1
            nib[top + y][left + x] = cnt.most_common(1)[0][0]
    open(out, 'wb').write(encode(nib))
    pv = Image.new('RGB', (560, 384))
    for y in range(192):
        for x in range(140):
            for dy in range(2):
                for dx in range(4):
                    pv.putpixel((x * 4 + dx, y * 2 + dy), PAL[nib[y][x]] if y < 160 else (0, 0, 0))
    pv.save(preview)
    print('logo', px, 'x', lines, 'at x', left)

if __name__ == '__main__':
    main(*sys.argv[1:4])
