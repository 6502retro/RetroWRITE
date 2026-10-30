#!/usr/bin/env python3
"""Preview a RetroWrite font at true Apple II double hi-res proportions."""
import importlib, sys
from PIL import Image

def render(font, text, width=520, bold=False, italic=False, scale=1):
    G = font.GLYPHS
    lines, cur, x = [], [], 0
    for word in text.split(' '):
        w = sum(len(G.get(c, G['?'])[0]) * scale + 1 + bold for c in word) + 4 * scale + 1
        if x + w > width and cur:
            lines.append(cur); cur, x = [], 0
        cur.append(word); x += w
    lines.append(cur)
    H = 11 * scale
    img = Image.new('1', (560, H * len(lines) + 4), 1)
    for li, words in enumerate(lines):
        x = 8
        for word in words + ['']:
            for c in (word + ' ') if word else '':
                rows = G.get(c, G['?'])
                gw = len(rows[0])
                for r, row in enumerate(rows):
                    sh = (7 - r) // 2 if italic else 0
                    for cx, ch in enumerate(row):
                        if ch == '#':
                            for dy in range(scale):
                                for dx in range(scale + bold):
                                    px = x + cx * scale + dx + sh
                                    py = 2 + li * H + r * scale + dy
                                    if px < 560:
                                        img.putpixel((px, py), 0)
                x += gw * scale + 1 + bold
    return img

def aspect(img):
    return img.convert('L').resize((560, int(img.size[1] * 2.19)), Image.NEAREST)

if __name__ == '__main__':
    sys.path.insert(0, sys.argv[1])
    font = importlib.import_module(sys.argv[2])
    text = ("The quick brown fox jumps over the lazy dog. THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG! "
            "MultiScribe had the feel of MacWrite: 0123456789 ($4.95) [x+y=z] {a*b} 50% & @home ~/a\\b <c> "
            "\"quoted\" it's #1; really? yes, really. jumpy quirky gypsy wizards vexed")
    parts = [render(font, text), render(font, 'Bold text looks like this, with smeared strokes.', bold=True),
             render(font, 'Italic text leans to the right like this.', italic=True),
             render(font, 'Large 18 point', scale=2)]
    h = sum(p.size[1] for p in parts)
    out = Image.new('1', (560, h), 1)
    y = 0
    for p in parts:
        out.paste(p, (0, y)); y += p.size[1]
    aspect(out).save(sys.argv[3])
