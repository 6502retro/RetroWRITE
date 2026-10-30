#!/usr/bin/env python3
"""Make RW2.SPLASH: 16K double hi-res image (8K aux half, then 8K main half)."""
import sys

def addr(y):
    return (y & 7) * 0x400 + ((y >> 3) & 7) * 0x80 + (y >> 6) * 0x28

def encode(nib):          # nib[y][x], 192 x 140, values 0..15 (bit pattern)
    aux = bytearray(8192); main = bytearray(8192)
    for y in range(192):
        bits = []
        for x in range(140):
            n = nib[y][x]
            bits += [(n >> i) & 1 for i in range(4)]
        o = addr(y)
        for j in range(80):
            b = 0
            for k in range(7):
                b |= bits[7 * j + k] << k
            (aux if j % 2 == 0 else main)[o + j // 2] = b
    return bytes(aux + main)

if __name__ == '__main__':
    if sys.argv[1] == '--calib':
        nib = [[(min(x // 8, 15) if x < 128 else 0) if y < 160 else 0 for x in range(140)] for y in range(192)]
        open(sys.argv[2], 'wb').write(encode(nib))
