"""Render dumped double hi-res memory as a black-on-white print (2x4 pixels per dot)."""
import sys, glob, os
from PIL import Image
def rowaddr(y): return (y & 7) * 0x400 + ((y >> 3) & 7) * 0x80 + (y >> 6) * 0x28
def render(f):
    m = open(f, 'rb').read(); main, aux = m[:0x2000], m[0x2000:0x4000]
    px = []
    for y in range(192):
        a = rowaddr(y)
        for c in range(40):
            for b in (aux[a + c], main[a + c]):
                for i in range(7): px.append(255 if (b >> i) & 1 else 0)
    im = Image.new('L', (560, 192)); im.putdata(px)
    return im
if __name__ == '__main__':
    for f in sys.argv[1:] or sorted(glob.glob(os.path.join(os.path.dirname(__file__), 'scr', '*.bin'))):
        render(f).resize((1120, 768), Image.NEAREST).save(f[:-4] + '.png')
