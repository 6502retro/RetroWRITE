#!/usr/bin/env python3
"""Write the RetroWrite II TUTORIAL document (RW2D version 3).

    python3 mktutorial.py OUT

Format bytes ($80 | f): bits 0-1 font (0 Sans, 1 Serif, 2 Mono), bits 2-3 size
(0 9pt, 1 18pt, 2 27pt, 3 superscript/subscript), $10 bold, $20 italic,
$40 underline. Paragraph markers: $83 | align<<2 | spacing<<4
(align 0 left, 1 center, 2 right, 3 justify; spacing 0 single, 1 1.5, 2 double).
"""
import struct, sys

B, I, U = 0x10, 0x20, 0x40
SERIF, MONO = 1, 2
S18, S27 = 4, 8
SUP, SUB = 0x0C, 0x0C | 0x10

out = bytearray(b'\x80')


def para(align=0, spacing=0):
    out.append(0x83 | align << 2 | spacing << 4)


def t(s, f=0):
    out.append(0x80 | f)
    out.extend(s.encode('ascii'))
    out.append(0x80)


def nl():
    out.append(0x0D)


def line(*parts, align=0, spacing=0):
    para(align, spacing)
    for p in parts:
        if isinstance(p, tuple):
            t(*p)
        else:
            t(p)
    nl()


OA = 'OA'      # the Open-Apple symbol is not part of the document fonts
line(('RetroWrite II', SERIF | S27 | B), align=1)
line(('A guided tour', SERIF | S18 | I), align=1)
line('')
line('This is a RetroWrite document, so everything in it - fonts, sizes, styles, the '
     'ruler and the centered headings - is kept when you save it. Try things out here: '
     'it opens as an untitled copy, so the original is never changed.', align=3)
line('')
line(('The mouse', S18 | B))
line(('Menus: ', B), 'press on a menu title, drag down to a command and let go. Or click a '
     'title once and the menu stays open until you click a command.')
line(('Typing place: ', B), 'click in the text to put the blinking caret there.')
line(('Selecting: ', B), 'drag across text. Double-click a word to select it; keep the '
     'button down after a double-click to select whole words. Hold an Apple key and click '
     'to extend a selection.')
line(('Scrolling: ', B), 'the bar at the right has arrows, a gray area that moves a page, '
     'and a box you can drag.')
line('')
line(('The keyboard', S18 | B))
line('Esc opens the menus from the keyboard: arrows to move, Return to choose. Every '
     'command that shows an Open-Apple key in its menu can be typed with it '
     '(OA below means hold Open-Apple):')
line(('  ' + OA + '-N new  ' + OA + '-O open  ' + OA + '-S save  ' + OA + '-P print  '
      + OA + '-Q quit', MONO))
line(('  ' + OA + '-X cut  ' + OA + '-C copy  ' + OA + '-V paste  ' + OA + '-W select all', MONO))
line(('  ' + OA + '-B bold  ' + OA + '-I italic  ' + OA + '-U underline  ' + OA + '-T plain', MONO))
line(('  ' + OA + '-F find  ' + OA + '-G again  ' + OA + '-R replace  ' + OA + '-K spelling', MONO))
line(('  ' + OA + '-arrows page and word moves   ' + OA + '-1 / ' + OA + '-9 top and end', MONO))
line('')
line(('Fonts, sizes and styles', S18 | B))
line('Choose a font and size from the Font and Size menus, and styles from the Style menu: ',
     ('bold', B), ', ', ('italic', I), ', ', ('underline', U), ', superscript as in E = mc',
     ('2', SUP), ' and subscript as in H', ('2', SUB), 'O. Fonts: Sans, ', ('Serif', SERIF),
     ' and ', ('Mono', MONO), '. With text selected, a choice changes the selection; '
     'with nothing selected it applies to what you type next.')
line('')
line(('The ruler', S18 | B))
line('Format > Show Ruler (' + OA + '-H) shows it. Drag the triangles to set the margins. '
     'Click on the scale to add a tab stop; drag a tab to move it, or down off the ruler '
     'to remove it. The boxes below the scale set the alignment and line spacing of the '
     'paragraphs that the caret or selection is in.')
line('This paragraph is centered.', align=1)
line('This one is set to the right.', align=2)
line('This paragraph has one and a half line spacing, which leaves more room between the '
     'lines when it is printed.', spacing=1)
line('')
line(('Printing', S18 | B))
line('File > Page Setup chooses the printer (ImageWriter II, Epson or plain text), the slot '
     'of the printer card, the quality and a header and footer. A # in a header or footer '
     'prints the page number. "As on screen" prints the fonts exactly as you see them; '
     '"Faster" uses the printer\'s own font. Print Several prints a list of documents one '
     'after another. Format > Insert Page Break starts a new page.')
line('')
line(('More than one document', S18 | B))
line('New and Open use a new window while the one on the screen has something in it, up '
     'to four. The Windows menu switches between them. The hidden ones wait on the hard '
     'disk, so any 128K Apple IIe can do this.')
line('')
line(('The glossary', S18 | B))
line('Press Control-G, then a key, to type a stored phrase. Edit > Edit Glossary opens the '
     'GLOSSARY file: each line is a key, an equal sign and the text, like  A=Apple IIe . '
     'Control-G then ? lists the keys.')
line('')
line(('Desk accessories', S18 | B))
line('The Apple menu has: Startup Preferences (ruler, the font for new documents, '
     'remembered printer settings and documents folder); Load DOS 3.3 File, which reads a '
     'text file from a DOS 3.3 disk in a 5.25" drive into your document; Communications, a '
     'terminal for a Super Serial Card that can keep what it receives; and BASIC.SYSTEM - '
     'type BYE in BASIC to come back.')
line('')
line(('Have fun writing!', SERIF | S18 | I), align=1)

head = b'RW2D' + bytes([3]) + struct.pack('<HH', 0, 512) + bytes(16)
head += b'RetroWrite II Tutorial'.ljust(40, b'\0') + b'Page #'.ljust(40, b'\0')
open(sys.argv[1], 'wb').write(head + out)
print(len(head + out), 'bytes')
