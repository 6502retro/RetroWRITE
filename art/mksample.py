#!/usr/bin/env python3
"""Write sample RetroWrite II documents used in the manual.

    python3 mksample.py OUTDIR      -> OUTDIR/LETTER, OUTDIR/MISSPELT
"""
import struct, sys, os

B, I, U = 0x10, 0x20, 0x40
SERIF, MONO = 1, 2
S18, S27 = 4, 8
SUP = 0x0C


def doc(build, tabs=(), head='', foot='', lm=0, rm=512):
    out = bytearray(b'\x80')

    def para(align=0, spacing=0):
        out.append(0x83 | align << 2 | spacing << 4)

    def t(s, f=0):
        out.append(0x80 | f)
        out.extend(s.encode('ascii'))
        out.append(0x80)

    def line(*parts, align=0, spacing=0):
        para(align, spacing)
        for p in parts:
            t(*p) if isinstance(p, tuple) else t(p)
        out.append(0x0D)

    build(line)
    tb = list(tabs) + [0] * (8 - len(tabs))
    h = b'RW2D' + bytes([3]) + struct.pack('<HH', lm, rm) + struct.pack('<8H', *tb)
    h += head.encode().ljust(40, b'\0') + foot.encode().ljust(40, b'\0')
    return h + out


def letter(line):
    line(('6502 Retro Computing', SERIF | S18 | B), align=1)
    line(('Apple II hardware and software since 2026', SERIF | I), align=1)
    line('')
    line('\t\t\tOctober 29, 2026')
    line('')
    line('Dear Club Members,')
    line('')
    line('Our next meeting is on ', ('Saturday', B), ' at the library. Bring your '
         'Apple IIe, a mouse and a hard disk: we will write the club newsletter with ',
         ('RetroWrite II', I), ' and print it on an ImageWriter II.', align=3)
    line('')
    line(('Agenda', U))
    line('\t1.\tWelcome and news')
    line('\t2.\tWriting with fonts, sizes and styles')
    line('\t3.\tPrinting the 2', ('nd', SUP), ' issue')
    line('')
    line('Yours sincerely,')
    line('')
    line(('The Newsletter Team', SERIF | I))


def misspelt(line):
    line('Teh newsletter is ready for printng. Please chek the spelling.')


if __name__ == '__main__':
    d = sys.argv[1]
    open(os.path.join(d, 'LETTER'), 'wb').write(
        doc(letter, tabs=(40, 80, 300), head='', foot='Page #'))
    open(os.path.join(d, 'MISSPELT'), 'wb').write(doc(misspelt))
    print('ok')
