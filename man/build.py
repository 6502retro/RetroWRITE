"""Typeset the RetroWrite II Owner's Manual as an 80s Apple-style PDF,
in the same design as the RetroDB and RetroCALC manuals.

Pass 1 renders with invisible markers to learn the page of every heading;
pass 2 renders the same layout with real page numbers in the contents.
Running footers are stamped on afterwards with reportlab."""
import re, html, os, sys, io
import markdown
from playwright.sync_api import sync_playwright
from pypdf import PdfReader, PdfWriter
import pdfplumber
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.colors import HexColor
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
FONTS = os.path.join(HERE, 'fonts')
IMG = os.path.join(HERE, 'figs')
OUT = sys.argv[1]
MD = open(os.path.join(HERE, 'manual.md')).read()

STRIPES = ['#77B957', '#F3BB44', '#E6883C', '#D04940', '#8B4292', '#4998D1']

lg = Image.open(os.path.join(IMG, 'logo-full.png')); lg.thumbnail((900, 900)); lg.save(f'{IMG}/logo.png')


# ------------------------------------------------------------- markdown
lines = MD.split('\n')
assert lines[0].startswith('# ')
body = '\n'.join(l for l in lines[1:] if not re.match(r'^\w{3} \d+, \d{4} · @', l))
parts = re.split(r'\n(?=## )', '\n' + body)
sections = []
for p in parts:
    p = p.strip('\n')
    if p.startswith('## '):
        title, _, rest = p.partition('\n')
        sections.append((title[3:].strip(), rest))
assert sections[0][0] == 'RetroWrite II 2.2'
front, chapters = sections[0], sections[1:]

KEYS = r'\b(RETURN|ESC|TAB|DELETE|RESET|UP ARROW|DOWN ARROW|LEFT ARROW|RIGHT ARROW|CONTROL-X|CONTROL)\b'


def md2html(text, figs):
    def img(m):
        alt, pct, src = m.group(1), int(m.group(2)), m.group(3)
        cap = f'<p class="cap">{html.escape(alt)}</p>' if alt.startswith('Figure') else ''
        return (f'\n<div class="fig"><img style="width:{pct}%" src="file://{IMG}/{src}">'
                f'{cap}</div>\n')
    text = re.sub(r'!\[(.*?)\|(\d+)\]\((.*?)\)', img, text)
    h = markdown.markdown(text, extensions=['tables', 'fenced_code', 'sane_lists'])

    def note(m):
        inner = m.group(1)
        kind = 'imp' if 'Important!' in inner else 'btw'
        return f'<div class="note {kind}"><div class="ico"></div>{inner}</div>'
    h = re.sub(r'<blockquote>\s*(.*?)\s*</blockquote>', note, h, flags=re.S)
    # key names in small capitals, outside code and tags
    out = []
    for chunk in re.split(r'(<code>.*?</code>|<pre>.*?</pre>|<[^>]+>)', h, flags=re.S):
        if chunk.startswith('<'):
            out.append(chunk)
        else:
            out.append(re.sub(KEYS, r'<span class="k">\1</span>', chunk))
    return ''.join(out)


def slug(s):
    return re.sub(r'[^a-z0-9]+', '-', s.lower()).strip('-')


def split_title(t):
    m = re.match(r'(Chapter \d+|Appendix [A-Z]|Preface): (.*)', t)
    if m:
        return m.group(1), m.group(2)
    return '', t


def css():
    ff = []
    for fam, w, st, f in [('EB Garamond', 400, 'normal', 'eb-garamond-latin-400-normal'),
                          ('EB Garamond', 400, 'italic', 'eb-garamond-latin-400-italic'),
                          ('EB Garamond', 500, 'normal', 'eb-garamond-latin-500-normal'),
                          ('EB Garamond', 600, 'normal', 'eb-garamond-latin-600-normal'),
                          ('EB Garamond', 700, 'normal', 'eb-garamond-latin-700-normal'),
                          ('Courier Prime', 400, 'normal', 'courier-prime-latin-400-normal'),
                          ('Courier Prime', 700, 'normal', 'courier-prime-latin-700-normal')]:
        ff.append(f"@font-face{{font-family:'{fam}';font-weight:{w};font-style:{st};src:url('file://{FONTS}/{f}.ttf');}}")
    return '\n'.join(ff) + open(os.path.join(HERE, 'style.css')).read()


def stripes(cls='stripes'):
    return f'<div class="{cls}">' + ''.join(f'<i style="background:{c}"></i>' for c in STRIPES) + '</div>'


def body_html(pages):
    """pages: {anchor: page number} or None (pass 1)"""
    toc_items = []
    chap_html = []
    for title, rest in chapters:
        label, name = split_title(title)
        a = slug(title)
        subs = re.findall(r'^### (.*)$', rest, flags=re.M)
        toc_items.append((label, name, a, [(s, a + '--' + slug(s)) for s in subs]))
        # markers for page finding
        def h3(m):
            s = m.group(1)
            return f'### {s} <span class="mk">@@{a}--{slug(s)}@@</span>'
        rest2 = re.sub(r'^### (.*)$', h3, rest, flags=re.M)
        lab = f'<div class="clabel">{label}</div>' if label else '<div class="clabel">&nbsp;</div>'
        chap_html.append(f'''<section class="chap" data-title="{html.escape(name)}">
<header class="opener">{lab}{stripes()}<h2>{html.escape(name)}</h2><span class="mk">@@{a}@@</span></header>
{md2html(rest2, True)}</section>''')

    def pn(a):
        return str(pages.get(a, '')) if pages else '00'
    toc = ['<section class="toc"><header class="opener"><div class="clabel">&nbsp;</div>' + stripes() +
           '<h2>Contents</h2><span class="mk">@@contents@@</span></header><div class="tocl">']
    for label, name, a, subs in toc_items:
        toc.append(f'<div class="tc"><span class="tl">{label}</span><span class="tn">{html.escape(name)}</span>'
                   f'<span class="dots"></span><span class="tp">{pn(a)}</span></div>')
        for s, sa in subs:
            toc.append(f'<div class="ts"><span class="tn">{html.escape(s)}</span><span class="dots"></span>'
                       f'<span class="tp">{pn(sa)}</span></div>')
    toc.append('</div></section>')

    fr = front[1]
    notice = fr.split('### Notice', 1)[1]
    copy = f'''<section class="copy"><div class="copyin">{stripes('stripes thin')}
<p class="cprod">RetroWrite II 2.2 Owner&rsquo;s Manual</p>
{md2html(notice, False)}
<p class="cfoot">&copy; 2026 Jonathan Adar (6502Retro). RetroWrite II is released under the MIT License.</p>
</div><span class="mk">@@copy@@</span></section>'''
    return f'''<!doctype html><html><head><meta charset="utf-8"><style>{css()}</style></head><body>
{copy}{''.join(toc)}{''.join(chap_html)}</body></html>''', toc_items


def cover_html():
    return f'''<!doctype html><html><head><meta charset="utf-8"><style>{css()} @page {{ margin: 0; }}</style></head><body class="coverbody">
<div class="cover">
  <div class="cband"><p class="capple">For the enhanced Apple IIe</p></div>
  <div class="cmid">
    <p class="ctitle">RetroWrite II</p>
    <p class="csub">Word Processor &middot; Version 2.2</p>
    <div class="monitor"><img src="file://{IMG}/splash.png"></div>
    <p class="cown">Owner&rsquo;s Manual</p>
  </div>
  {stripes('cstripes')}
</div></body></html>'''


def back_html():
    return f'''<!doctype html><html><head><meta charset="utf-8"><style>{css()} @page {{ margin: 0; }}</style></head><body class="coverbody">
<div class="back">
  {stripes('cstripes top')}
  <div class="bmid"><img class="logo" src="file://{IMG}/logo.png">
  <p>RetroWrite II 2.2 &middot; a word processor for the Apple IIe</p>
  <p class="small">6502Retro &middot; Retro Computing</p></div>
</div></body></html>'''


def render(page, htmltext, path):
    tmp = os.path.join(HERE, 'tmp.html')
    open(tmp, 'w').write(htmltext)
    page.goto('file://' + tmp)
    page.wait_for_timeout(400)
    page.pdf(path=path, format='Letter', print_background=True, prefer_css_page_size=True)


def roman(n):
    s = ''
    for v, r in [(10, 'x'), (9, 'ix'), (5, 'v'), (4, 'iv'), (1, 'i')]:
        while n >= v:
            s += r; n -= v
    return s


def find_marks(path):
    where = {}
    with pdfplumber.open(path) as pdf:
        for i, page in enumerate(pdf.pages):
            t = (page.extract_text() or '').replace('\n', '')
            for m in re.findall(r'@@([a-z0-9-]+)@@', t):
                where.setdefault(m, i)
        return where, len(pdf.pages)


with sync_playwright() as p:
    b = p.chromium.launch()
    pg = b.new_page()
    h1, toc_items = body_html(None)
    render(pg, h1, os.path.join(HERE, 'pass1.pdf'))
    where, n1 = find_marks(os.path.join(HERE, 'pass1.pdf'))
    first = where[toc_items[1][2]]        # Chapter 1 is page 1; the preface before it
    pages = {a: (i - first + 1) for a, i in where.items()}
    # preface pages are front matter: roman numerals
    for a, i in where.items():
        if i < first:
            pages[a] = roman(i + 1)
    h2, _ = body_html(pages)
    render(pg, h2, os.path.join(HERE, 'pass2.pdf'))
    where2, n2 = find_marks(os.path.join(HERE, 'pass2.pdf'))
    assert where2 == where, 'pages moved between passes'
    render(pg, cover_html(), os.path.join(HERE, 'cover.pdf'))
    render(pg, back_html(), os.path.join(HERE, 'back.pdf'))
    b.close()

# ------------------------------------------------------------- footers
pdfmetrics.registerFont(TTFont('Gar', f'{FONTS}/eb-garamond-latin-400-normal.ttf'))
pdfmetrics.registerFont(TTFont('GarI', f'{FONTS}/eb-garamond-latin-400-italic.ttf'))
starts = sorted((where[a], split_title(t)) for t, a in [(c[0], slug(c[0])) for c in chapters])
body = PdfReader(os.path.join(HERE, 'pass2.pdf'))
w = PdfWriter()
w.add_page(PdfReader(os.path.join(HERE, 'cover.pdf')).pages[0])
for i, page in enumerate(body.pages):
    W = float(page.mediabox.width); H = float(page.mediabox.height)
    buf = io.BytesIO(); c = canvas.Canvas(buf, pagesize=(W, H))
    name = ''
    for s, (lab, nm) in starts:
        if s <= i:
            name = nm
    opener = any(s == i for s, _ in starts) or i == where.get('contents')
    if i > 0:
        num = roman(i + 1) if i < first else str(i - first + 1)
        c.setFillColor(HexColor('#1d1d1d'))
        right = (i - first) % 2 == 0 if i >= first else i % 2 == 0
        y = 0.55 * 72
        c.setFont('Gar', 10.5)
        if right:
            c.drawRightString(W - 0.75 * 72, y, num)
            if not opener and name:
                c.setFont('GarI', 10); c.drawRightString(W - 0.75 * 72 - 26, y, name)
        else:
            c.drawString(0.75 * 72, y, num)
            if not opener and name:
                c.setFont('GarI', 10); c.drawString(0.75 * 72 + 26, y, name)
    c.showPage(); c.save(); buf.seek(0)
    page.merge_page(PdfReader(buf).pages[0])
    w.add_page(page)
w.add_page(PdfReader(os.path.join(HERE, 'back.pdf')).pages[0])
w.add_metadata({'/Title': "RetroWrite II Owner's Manual", '/Author': '6502Retro'})
w.write(OUT)
print('pages', len(w.pages), 'chapter 1 at pdf page', first + 2)
