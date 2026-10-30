"""Page design for the RetroWrite II manual, after the Mouse Calc (Version Soft,
1984) manual: condensed bold-italic headings on yellow bars, reference entries
with Definition / What to do / Example / NOTE / See, black-on-white
screenshots and yellow page-number boxes."""
import os
from reportlab.lib.colors import Color, black, white
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_LEFT, TA_CENTER, TA_JUSTIFY
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (Flowable, Paragraph, Table, TableStyle, Spacer, Image,
                                KeepTogether, CondPageBreak)
from reportlab.platypus.tableofcontents import TableOfContents, SimpleIndex

W, H = 595.32, 841.92                   # A4, like the Mouse Calc PDF
YEL = Color(252 / 255, 238 / 255, 40 / 255)
GREY = Color(.62, .62, .62)
LIGHT = Color(.90, .90, .90)
FD = '/usr/share/fonts/truetype/dejavu/'
pdfmetrics.registerFont(TTFont('Head', FD + 'DejaVuSansCondensed-BoldOblique.ttf'))
pdfmetrics.registerFont(TTFont('HeadR', FD + 'DejaVuSansCondensed-Bold.ttf'))

# text column (pages are two-sided; the column is centred so both sides work)
X0, X1 = 92, 503
COLW = X1 - X0
LABW = 74                               # Definition / What to do ... column

BODY = ParagraphStyle('body', fontName='Helvetica', fontSize=9, leading=11.6, spaceAfter=4)
BODYJ = ParagraphStyle('bodyj', parent=BODY, alignment=TA_JUSTIFY)
NOTE = ParagraphStyle('note', parent=BODY, fontName='Helvetica-Oblique', fontSize=8.6, leading=11)
STEP = ParagraphStyle('step', parent=BODY, leftIndent=13, firstLineIndent=-13, spaceAfter=2.5)
BUL = ParagraphStyle('bul', parent=BODY, leftIndent=10, firstLineIndent=-8, spaceAfter=2)
CAP = ParagraphStyle('cap', fontName='Helvetica-Oblique', fontSize=7.6, leading=9, alignment=TA_CENTER)
SMALL = ParagraphStyle('small', parent=BODY, fontSize=7.8, leading=9.6)
LAB = ParagraphStyle('lab', fontName='Head', fontSize=9.4, leading=11.6)
CELL = ParagraphStyle('cell', parent=BODY, fontSize=8.4, leading=10.4, spaceAfter=0)
CELLB = ParagraphStyle('cellb', parent=CELL, fontName='Helvetica-Bold')


def scr(s):
    """text you type or see on the screen"""
    return '<font face="Courier-Bold" size="8.6">%s</font>' % s


MAN = os.path.dirname(os.path.abspath(__file__))
OAIMG = os.path.join(MAN, 'oapple.png')
SAIMG = os.path.join(MAN, 'sapple.png')


def oa(k=None):
    """the Open-Apple key (drawn from RetroWrite II's own font), optionally with a key"""
    img = '<img src="%s" width="5.6" height="7.4" valign="-1"/>' % OAIMG
    return img if k is None else '<b>%s-%s</b>' % (img, k)


def sa():
    return '<img src="%s" width="5.6" height="7.4" valign="-1"/>' % SAIMG


def head_text(c, x, y, text, size, scale=82, font='Head'):
    t = c.beginText(x, y)
    t.setFont(font, size)
    t.setHorizScale(scale)
    t.textOut(text)
    c.drawText(t)


def head_width(text, size, scale=82, font='Head'):
    return pdfmetrics.stringWidth(text, font, size) * scale / 100


def draw_keys(c, x, y, text, size=7.6):
    """draw text where '@' is the Open-Apple key"""
    parts = text.split('@')
    for i, p in enumerate(parts):
        if i:
            c.drawImage(OAIMG, x, y - 1, 5.0, 6.6, mask='auto')
            x += 5.8
        c.setFont('Helvetica', size)
        c.drawString(x, y, p)
        x += pdfmetrics.stringWidth(p, 'Helvetica', size)
    return x


class Bookmark(Flowable):
    """an invisible TOC / outline entry"""
    def __init__(self, title, level, key):
        super().__init__()
        self.title, self.level, self.key = title, level, key
        self.width = self.height = 0

    def draw(self):
        self.canv.bookmarkPage(self.key)
        self.canv.addOutlineEntry(self.title, self.key, level=self.level)


class EntryHead(Flowable):
    """reference entry heading: yellow bar, condensed bold italic title, and a
    MENU / KEYS line under a grey rule"""
    def __init__(self, title, menu=None, keys=None, size=14):
        super().__init__()
        self.title, self.menu, self.keys, self.size = title, menu, keys, size
        self.width = COLW
        self.height = self.size + 10 + (14 if (menu or keys) else 2)

    def wrap(self, aw, ah):
        return self.width, self.height

    def draw(self):
        c = self.canv
        top = self.height
        c.setFillColor(YEL)
        c.rect(-8, top - self.size - 6, 8 + head_width(self.title, self.size) + 14, self.size + 6, stroke=0, fill=1)
        c.setFillColor(black)
        head_text(c, 0, top - self.size + 0.5, self.title, self.size)
        if self.menu or self.keys:
            y = top - self.size - 17
            c.setStrokeColor(GREY)
            c.setLineWidth(0.6)
            c.line(0, y + 11, self.width, y + 11)
            x = 0
            c.setFont('Helvetica-Bold', 7.2)
            if self.menu:
                c.drawString(x, y + 2, 'MENU:')
                c.setFont('Helvetica', 7.6)
                c.drawString(x + 30, y + 2, self.menu)
                x = 150
            if self.keys:
                c.setFont('Helvetica-Bold', 7.2)
                c.drawString(x, y + 2, 'KEYS:')
                draw_keys(c, x + 28, y + 2, self.keys)


class SubHead(Flowable):
    """a smaller heading inside a chapter (no bar, yellow tick at the left)"""
    def __init__(self, title, size=12):
        super().__init__()
        self.title, self.size = title, size
        self.height = size + 7

    def wrap(self, aw, ah):
        return COLW, self.height

    def draw(self):
        c = self.canv
        c.setFillColor(YEL)
        c.rect(-14, 1, 8, self.size + 1, stroke=0, fill=1)
        c.setFillColor(black)
        head_text(c, 0, 3, self.title, self.size)


class ChapterHead(Flowable):
    """chapter opener: a wide yellow band with the chapter number and title"""
    def __init__(self, num, title):
        super().__init__()
        self.num, self.title = num, title
        self.height = 96

    def wrap(self, aw, ah):
        return COLW, self.height

    def draw(self):
        c = self.canv
        c.setFillColor(YEL)
        c.rect(-X0, 30, W, 58, stroke=0, fill=1)
        c.setFillColor(black)
        if self.num:
            head_text(c, 0, 44, self.num, 34)
            x = head_width(self.num, 34) + 14
        else:
            x = 0
        head_text(c, x, 44, self.title, 28)
        c.setStrokeColor(black)
        c.setLineWidth(1.2)
        c.line(0, 22, COLW, 22)


def section(label, *flow):
    """one Definition / What to do / Example / NOTE / See row"""
    parts = []
    for f in flow:
        if isinstance(f, str):
            parts.append(Paragraph(f, NOTE if label == 'NOTE' else BODY))
        else:
            parts.append(f)
    t = Table([[Paragraph(label, LAB), parts]], colWidths=[LABW, COLW - LABW])
    t.setStyle(TableStyle([('VALIGN', (0, 0), (-1, -1), 'TOP'),
                           ('LEFTPADDING', (0, 0), (-1, -1), 0),
                           ('RIGHTPADDING', (0, 0), (-1, -1), 0),
                           ('TOPPADDING', (0, 0), (-1, -1), 1),
                           ('BOTTOMPADDING', (0, 0), (-1, -1), 3)]))
    return t


def steps(*items):
    return [Paragraph('<b>%d.</b>&nbsp;&nbsp;%s' % (i + 1, s), STEP) for i, s in enumerate(items)]


def bullets(*items, style=BUL):
    return [Paragraph('•&nbsp;%s' % s, style) for s in items]


def table(rows, widths, head=True, size=None):
    data = []
    for i, r in enumerate(rows):
        data.append([Paragraph(str(x), CELLB if (head and i == 0) else CELL) for x in r])
    t = Table(data, colWidths=widths, repeatRows=1 if head else 0)
    st = [('VALIGN', (0, 0), (-1, -1), 'TOP'),
          ('LINEBELOW', (0, 0), (-1, -1), 0.4, GREY),
          ('LEFTPADDING', (0, 0), (-1, -1), 3), ('RIGHTPADDING', (0, 0), (-1, -1), 3),
          ('TOPPADDING', (0, 0), (-1, -1), 2), ('BOTTOMPADDING', (0, 0), (-1, -1), 3)]
    if head:
        st += [('BACKGROUND', (0, 0), (-1, 0), YEL), ('LINEBELOW', (0, 0), (-1, 0), 0.8, black)]
    t.setStyle(TableStyle(st))
    return t


class Shot(Flowable):
    """a black-on-white screenshot with a thin frame; optional crop (in Apple
    screen pixels x0, y0, x1, y1) and callouts [(x, y, text, side)]"""
    def __init__(self, path, width, crop=None, callouts=(), lab_w=96):
        super().__init__()
        from PIL import Image as PImage
        im = PImage.open(path)
        if crop:
            x0, y0, x1, y1 = crop
            im = im.crop((x0 * 2, y0 * 4, x1 * 2, y1 * 4))
            self.ax0, self.ay0, self.ax1, self.ay1 = crop
        else:
            self.ax0, self.ay0, self.ax1, self.ay1 = 0, 0, 560, 192
        self.im = im
        self.callouts = callouts
        self.lab_w = lab_w if callouts else 0
        self.w = width
        self.h = width * im.height / im.width
        self.width = COLW
        self.height = self.h + 2

    def wrap(self, aw, ah):
        return self.width, self.height

    def draw(self):
        from reportlab.lib.utils import ImageReader
        import io
        c = self.canv
        x = (COLW - self.w) / 2
        buf = io.BytesIO()
        self.im.save(buf, 'PNG')
        buf.seek(0)
        c.drawImage(ImageReader(buf), x, 1, self.w, self.h)
        c.setStrokeColor(black)
        c.setLineWidth(0.6)
        c.rect(x, 1, self.w, self.h, stroke=1, fill=0)
        sx = self.w / (self.ax1 - self.ax0)
        sy = self.h / (self.ay1 - self.ay0)
        pts = {'L': [], 'R': []}
        for (ax, ay, text, side) in self.callouts:
            pts[side].append([x + (ax - self.ax0) * sx, 1 + self.h - (ay - self.ay0) * sy, text])
        for side, lst in pts.items():
            lst.sort(key=lambda p: -p[1])          # top first
            ys = [p[1] for p in lst]
            for i in range(1, len(ys)):            # push labels apart, at least 9pt
                ys[i] = min(ys[i], ys[i - 1] - 9)
            lo = 1 + 3                              # keep inside the picture's height
            if ys and ys[-1] < lo:
                d = lo - ys[-1]
                ys = [y + d for y in ys]
                for i in range(len(ys) - 2, -1, -1):
                    ys[i] = max(ys[i], ys[i + 1] + 9)
            edge = x if side == 'L' else x + self.w
            lx = x - 14 if side == 'L' else x + self.w + 14
            for (px, py, text), ly in zip(lst, ys):
                c.setStrokeColor(black)
                c.setLineWidth(0.5)
                p = c.beginPath()
                p.moveTo(px, py)
                p.lineTo(edge, py)
                p.lineTo(lx, ly)
                c.drawPath(p, stroke=1, fill=0)
                c.setFillColor(black)
                c.circle(px, py, 1.3, stroke=0, fill=1)
                c.setFont('Helvetica-Oblique', 7.4)
                if side == 'L':
                    c.drawRightString(lx - 2, ly - 2.5, text)
                else:
                    c.drawString(lx + 2, ly - 2.5, text)


def shot(path, caption=None, width=300, crop=None, callouts=()):
    items = [Spacer(1, 4), Shot(path, width, crop, callouts)]
    if caption:
        items += [Spacer(1, 3), Paragraph(caption, CAP)]
    items.append(Spacer(1, 8))
    return KeepTogether(items)
