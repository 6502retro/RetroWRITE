# RetroWrite II "Serif" - Sans with slab serifs added to the ends of straight
# vertical stems, plus a few hand-drawn letters.
import font_sans
from font_sans import g, B

def add_serifs(rows):
    h = len(rows)
    w = len(rows[0])
    grid = [list(B + r + B) for r in rows]           # one column of room each side
    W = w + 2
    def px(r, c):
        return 0 <= r < h and 0 <= c < W and grid[r][c] == '#'
    adds = []
    for r in range(h):
        c = 0
        while c < W - 1:
            if px(r, c) and px(r, c + 1) and not px(r, c - 1) and not px(r, c + 2):
                # vertical stem pair: bottom end?
                if px(r - 1, c) and px(r - 1, c + 1) and px(r - 2, c) and px(r - 2, c + 1) \
                        and not px(r + 1, c) and not px(r + 1, c + 1) and r in (7, 9):
                    adds.append((r, c - 1)); adds.append((r, c + 2))
                # top end
                if px(r + 1, c) and px(r + 1, c + 1) and px(r + 2, c) and px(r + 2, c + 1) \
                        and not px(r - 1, c) and not px(r - 1, c + 1) and r in (1, 3):
                    adds.append((r, c - 1))
                    if r == 1:
                        adds.append((r, c + 2))
                c += 2
            else:
                c += 1
    for r, c in adds:
        grid[r][c] = '#'
    out = [''.join(r) for r in grid]
    # trim empty edge columns
    while all(r[0] == B for r in out):
        out = [r[1:] for r in out]
    while all(r[-1] == B for r in out):
        out = [r[:-1] for r in out]
    return out

hand = {
'O': g('..#####..', '.##...##.', '##.....##', '##.....##', '##.....##', '.##...##.', '..#####..'),
'o': g('.#####.', '##...##', '##...##', '##...##', '.#####.', top=3),
'S': g('.#####.#', '##....##', '###.....', '.######.', '.....###', '##....##', '#.#####.'),
'C': g('..#####.#', '.##....##', '##.......', '##.......', '##.......', '.##....##', '..#####..'),
'a': g('.#####..', '.....##.', '.######.', '##...##.', '.#######', top=3),
'e': g('.#####.', '##...##', '#######', '##.....', '.######', top=3),
'f': g('...###', '..##..', '.####.', '..##..', '..##..', '..##..', '.####.'),
't': g('.##..', '#####', '.##..', '.##..', '.##..', '..###', top=2),
'1': g('..##..', '.###..', '..##..', '..##..', '..##..', '..##..', '.####.'),
'j': g('...##', '.....', '..###', '...##', '...##', '...##', '...##', '...##', '####.'),
}

GLYPHS = {}
for ch, rows in font_sans.GLYPHS.items():
    if ch in hand:
        GLYPHS[ch] = hand[ch]
    elif ch.isalpha():
        GLYPHS[ch] = add_serifs(rows)
    else:
        GLYPHS[ch] = rows
NAME = 'Serif'
