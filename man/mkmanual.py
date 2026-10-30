#!/usr/bin/env python3
"""Build the RetroWrite II user's manual (PDF).

    python3 mkmanual.py OUT.pdf
"""
import os, sys
from reportlab.platypus import (BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer,
                                PageBreak, KeepTogether, CondPageBreak, NextPageTemplate,
                                Table, TableStyle, Image)
from reportlab.platypus.tableofcontents import TableOfContents, SimpleIndex
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.colors import black, white
from style import *

S = os.path.join(MAN, 'scr')
ART = os.path.join(MAN, '..', 'art')


def P(t, st=BODY):
    return Paragraph(t, st)


def ix(*terms):
    """index terms (attached to the next paragraph)"""
    return ''.join('<index item="%s"/>' % t for t in terms)


def scrn(name):
    return os.path.join(S, name + '.png')


story = []
A = story.append
_chapter = ['']
_toc_keys = [0]


def chapter(num, title, intro=None):
    A(NextPageTemplate('body'))
    A(PageBreak())
    _toc_keys[0] += 1
    key = 'c%d' % _toc_keys[0]
    full = ('%s  %s' % (num, title)) if num else title
    b = Bookmark(full, 0, key)
    b.toc = (0, full, key)
    b.chapter = title
    A(b)
    A(ChapterHead(num, title))
    if intro:
        for p in intro:
            A(P(p, BODYJ))


def head(title, level=1):
    _toc_keys[0] += 1
    key = 's%d' % _toc_keys[0]
    b = Bookmark(title, 1, key)
    b.toc = (level, title, key)
    return b


def entry(title, menu=None, keys=None, *rows, index=None):
    """a reference entry; rows are (label, content...) tuples"""
    items = [CondPageBreak(120), head(title), EntryHead(title, menu, keys), Spacer(1, 4)]
    first = True
    for r in rows:
        label, *content = r
        if first and index is not False:
            terms = index or [title]
            if content and isinstance(content[0], str):
                content[0] = ix(*terms) + content[0]
            first = False
        items.append(section(label, *content))
    items.append(Spacer(1, 10))
    story.extend(items)


def sub(title, toc=True):
    A(CondPageBreak(90))
    if toc:
        A(head(title, 1))
    A(Spacer(1, 4))
    A(SubHead(title))
    A(Spacer(1, 4))


# ============================================================ front matter
A(Spacer(1, 1))          # the cover is drawn by the cover page template

A(NextPageTemplate('plain'))
A(PageBreak())
A(Spacer(1, 40))
A(P('<font name="Head" size="16">RetroWrite II</font>', BODY))
A(P('Word processing for the enhanced Apple IIe', BODY))
A(Spacer(1, 10))
A(P('User’s Manual for version 2.1, 2026', BODY))
A(Spacer(1, 300))
A(P('RetroWrite II and this manual were written by 6502 Retro Computing in 2026 for '
    'Apple II enthusiasts. You may copy the program and this manual for your own use '
    'and share them with other Apple II users, free of charge.', SMALL))
A(P('Apple, Apple IIe, ProDOS, ImageWriter and Apple II DeskTop are names belonging to '
    'their owners. RetroWrite II is an independent program; it is not made or endorsed '
    'by Apple or by the publishers of MultiScribe, MouseWrite or MacWrite, whose way of '
    'working it follows.', SMALL))
A(P('Every screen picture in this manual was taken from RetroWrite II itself, running '
    'on an emulated Apple IIe.', SMALL))

A(PageBreak())
A(ChapterHead('', 'Contents'))
toc = TableOfContents(dotsMinLevel=0)
toc.levelStyles = [
    ParagraphStyle('t0', fontName='Head', fontSize=11.5, leading=17, spaceBefore=5),
    ParagraphStyle('t1', fontName='Helvetica', fontSize=8.8, leading=11.2, leftIndent=16),
]
A(toc)

# ============================================================ introduction
chapter('', 'Welcome to RetroWrite II', [
    'RetroWrite II is a word processor for the enhanced Apple IIe, in the tradition of '
    'MacWrite, MultiScribe and MouseWrite. What you see on the screen is what you get on '
    'paper: three fonts, three sizes, bold, italic, underline, superscript and subscript, '
    'centered and justified paragraphs, tab stops and page breaks all show as you type, '
    'and print the same way on an ImageWriter II or an Epson printer.',
    'You work with the mouse and pull-down menus, or with the keyboard: every menu can be '
    'opened with the Esc key, and the most used commands have an %s-key shortcut. '
    'RetroWrite II is made to live on a hard disk; the parts of the program that you '
    'use less often are read from the disk when you need them, which leaves room in '
    'memory for long documents.' % oa(),
])
sub('What you need')
for t in bullets(
        'An enhanced Apple IIe (65C02) with 128K of memory (an extended 80-column card). It '
        'should also run on an Apple IIc or IIgs, but it was made and tested on the IIe.',
        'An AppleMouse or compatible mouse card: you need the mouse to select text, although '
        'every command can also be given from the keyboard.',
        'A hard disk, or a large disk image, with ProDOS 8. RetroWrite II takes about 600K '
        'including its dictionary.',
        'A monochrome monitor gives the sharpest text. A color monitor also works.',
        'For printing: an ImageWriter II or an Epson-compatible printer on a parallel or serial '
        'printer card.'):
    A(t)
sub('About this manual')
A(P('Chapter 1 tells you how to install and start RetroWrite II and introduces the screen. '
    'Chapter 2 is a short tour: you write, style and print a letter. Chapters 3 to 11 are '
    'the reference section, one chapter per menu, with an entry for every command. The '
    'appendices list every key, the files RetroWrite II uses, its limits and its messages, '
    'and there is an index at the back.', BODYJ))
A(P('Each reference entry has the same parts, as in the Version Soft manuals:', BODY))
A(table([['Part', 'What it tells you'],
         ['Definition', 'what the command does'],
         ['What to do', 'the steps, with the mouse or the keyboard'],
         ['Example', 'a short example'],
         ['NOTE', 'things to know, limits, tips'],
         ['See', 'related entries']], [90, COLW - 90]))
A(Spacer(1, 6))
A(P('Words in %s are what you type or what appears on the screen. %s is the '
    'Open-Apple key, at the left of the space bar: %s means hold down %s and press S. '
    'The Solid-Apple key (%s, at the right of the space bar) works the same way.'
    % (scr('this type'), oa(), oa('S'), oa(), sa()), BODYJ))

# ============================================================ 1 getting started
chapter('1', 'Getting Started', [
    'This chapter explains how to install RetroWrite II on your hard disk, how to start '
    'it, and what you see on the screen.'])
sub('Installing RetroWrite II')
A(P(ix('installing') + 'RetroWrite II comes installed on the disk image it was delivered '
    'on, in the folder %s. To put it on another hard disk, copy the whole '
    '%s folder there, for example with Apple II DeskTop. Everything RetroWrite II needs '
    'is in that folder; it finds the folder by itself wherever you put it.'
    % (scr('/DESKTOP/RETROWRITE'), scr('RETROWRITE')), BODYJ))
A(P('On a PC you can also use the included installer, which copies the files into a disk '
    'image and adds RetroWrite II to Apple II DeskTop’s Shortcuts menu:', BODY))
A(P(scr('python3 install_rw2.py MYDISK.po --ac ac.jar --files .'), BODY))
sub('Starting RetroWrite II')
A(P(ix('starting RetroWrite II', 'Apple II DeskTop') + 'In Apple II DeskTop, open the '
    '%s folder and double-click %s, or choose RetroWrite II from the Shortcuts menu. '
    'From BASIC.SYSTEM type %s; from a program selector such as Bitsy Bye, choose '
    '%s.' % (scr('RETROWRITE'), scr('RW2.SYSTEM'), scr('-/DESKTOP/RETROWRITE/RW2.SYSTEM'),
             scr('RW2.SYSTEM')), BODYJ))
A(P('A welcome picture may appear for a few seconds while the program loads; press any '
    'key to go on. Then you see an empty, untitled document ready for typing.', BODYJ))
A(P(ix('/RAM') + 'RetroWrite II uses all of the Apple IIe’s auxiliary memory, so '
    'it switches off the /RAM disk while it runs and switches it back on when you quit. '
    'If /RAM has files in it, RetroWrite II warns you first and lets you stop.', NOTE))

sub('The RetroWrite II screen')
A(P(ix('screen') + 'Here is RetroWrite II with a letter on the screen and the ruler '
    'showing:', BODY))
A(shot(scrn('letter'), 'The RetroWrite II screen', width=290, callouts=[
    (6, 4, 'Apple menu', 'L'), (120, 4, 'menu bar', 'L'),
    (125, 18, 'ruler', 'L'), (14, 23, 'left margin', 'L'),
    (20, 32, 'alignment', 'L'), (110, 32, 'line spacing', 'L'),
    (150, 115, 'your text', 'L'), (360, 92, 'tab stop', 'R'),
    (521, 23, 'right margin', 'R'), (549, 16, 'scroll arrow', 'R'),
    (549, 40, 'scroll box', 'R'), (549, 120, 'gray area', 'R'),
    (470, 175, 'pointer (I-beam)', 'R')]))
A(P('<b>The menu bar</b> holds the menus: the Apple menu (%s), File, Edit, Search, '
    'Format, Font, Size, Style and Windows. <b>The ruler</b> (shown with %s) sets '
    'margins, tab stops, alignment and line spacing. <b>The text area</b> shows your '
    'document as it will print. <b>The scroll bar</b> at the right moves through a long '
    'document. <b>The pointer</b> follows the mouse: an arrow over the menus and the '
    'scroll bar, an I-beam over the text. <b>The caret</b>, a blinking line, shows '
    'where the next letter you type goes.' % (sa(), oa('H')), BODYJ))

sub('Using the mouse')
A(P(ix('mouse', 'clicking', 'dragging', 'pull-down menus') +
    'RetroWrite II works like the programs you may know from the Macintosh and the '
    'Apple IIgs:', BODY))
for t in bullets(
        '<b>Point</b> by moving the mouse. <b>Click</b>: press and release the button. '
        '<b>Double-click</b>: click twice quickly. <b>Drag</b>: hold the button down while '
        'you move the mouse.',
        '<b>Choosing from a menu:</b> point at a menu title, hold the button down, drag '
        'down to the command and let go. Or click the title once: the menu stays open '
        'until you click a command (or click outside to close it). Commands you can’t '
        'use are dimmed; a check mark shows a setting that is on.',
        '<b>Placing the caret:</b> click in the text.',
        '<b>Selecting text:</b> drag across it. Double-click a word to select the word; '
        'keep the button down after the second click and drag to select whole words. '
        'Hold down %s or %s and click to extend a selection to the place you click.'
        % (oa(), sa()),
        '<b>Scrolling:</b> click a scroll arrow to move one line (hold it down to keep '
        'scrolling), click the gray area to move a page, or drag the scroll box.'):
    A(t)
sub('Using the keyboard')
A(P(ix('keyboard', 'Esc key') + 'Type as on a typewriter. You never press Return at '
    'the end of a line: RetroWrite II moves words to the next line by itself (word '
    'wrap). Press Return only to end a paragraph. The arrow keys move the caret; '
    'Delete erases the character before the caret, or the selected text.', BODYJ))
A(P('<b>Esc opens the menus from the keyboard.</b> The File menu opens first; use the '
    'left and right arrows to move to another menu, the up and down arrows to choose a '
    'command, and Return to give it. Typing the first letter of a command also gives it. '
    'Press Esc again to close the menu. Appendix A lists every key.', BODYJ))
A(shot(scrn('m_file_sel'), 'The File menu, opened with Esc, with Open... chosen with the arrow keys',
       width=300, crop=(0, 0, 300, 120)))
sub('Leaving RetroWrite II')
A(P(ix('quitting') + 'Choose Quit from the File menu (%s). If a document has changes '
    'you haven’t saved, RetroWrite II asks whether to save them. You return to the '
    'program you started RetroWrite II from, such as Apple II DeskTop.' % oa('Q'), BODYJ))

# ============================================================ 2 tour
chapter('2', 'A Quick Tour', [
    'In this chapter you write the short club letter shown below, give it fonts and '
    'styles, set its tabs, save it and print it. It takes about fifteen minutes. '
    'The finished letter is also in the %s folder, as %s, if you want to look at it '
    'first.' % (scr('DOCUMENTS'), scr('LETTER'))])
A(shot(scrn('letter'), 'The finished letter', width=290))
sub('Step 1: Type the text')
A(P('Start RetroWrite II; an empty document appears. Type the first line, ' +
    scr('6502 Retro Computing') + ', and press Return. Type the second line and press '
    'Return twice. Then type the rest of the letter. Don’t press Return inside a '
    'paragraph: let the words wrap by themselves. Press Return at the end of each '
    'paragraph and once more for an empty line.', BODYJ))
A(P('If you make a mistake, press Delete to erase back to it, or click just after the '
    'mistake, press Delete and type the correction.', BODYJ))
sub('Step 2: Center the heading')
A(P('Click anywhere in the first line and choose <b>Center</b> from the Format menu '
    '(%s). Do the same for the second line. Alignment belongs to the whole paragraph, so '
    'you don’t need to select the text.' % oa('E'), BODYJ))
sub('Step 3: Change fonts, sizes and styles')
A(P('Drag across the first line to select it. Choose <b>Serif</b> from the Font menu, '
    '<b>18 Point</b> from the Size menu and <b>Bold</b> from the Style menu (%s). Select '
    'the second line and choose Serif and <b>Italic</b> (%s).' % (oa('B'), oa('I')), BODYJ))
A(P('Double-click the word %s and press %s to make it bold. Select the word %s and '
    'choose <b>Underline</b> (%s).' % (scr('Saturday'), oa('B'), scr('Agenda'), oa('U')), BODYJ))
A(P('You can also choose the style before you type: with nothing selected, choose Bold, '
    'type the words, and choose <b>Plain Text</b> (%s) to go back to normal letters.'
    % oa('T'), BODYJ))
A(shot(scrn('m_style'), 'The Style menu. The check mark shows the style of the selected text.',
       width=300, crop=(260, 0, 560, 110)))
sub('Step 4: Set tab stops for the date and the agenda')
A(P('Choose <b>Show Ruler</b> from the Format menu (%s). Click the ruler’s scale at '
    '½ inch and at 1 inch to put two tab stops there, and at about 3¾ inches '
    'for the date. Now click at the start of the date, press Tab three times, and the '
    'date moves to the third stop. In the agenda lines, press Tab before the number and '
    'again after it.' % oa('H'), BODYJ))
A(P('Click in the long paragraph and click the fourth alignment icon in the ruler '
    '(or choose <b>Justify</b>, %s) so that both of its edges are straight.' % oa('J'), BODYJ))
sub('Step 5: Save the letter')
A(P('Choose <b>Save</b> from the File menu (%s). Because the letter has no name yet, the '
    'Save dialog appears. Type a name, such as %s, and click <b>Save</b>. The name '
    'appears at the top of the File menu and in the Windows menu.'
    % (oa('S'), scr('MYLETTER')), BODYJ))
sub('Step 6: Print it')
A(P('Turn on your printer and choose <b>Print</b> from the File menu (%s). The first '
    'time, click <b>Setup...</b> and choose your printer and the slot of its card, then '
    'click OK and choose Print again. RetroWrite II shows the page it is printing; '
    'press Esc to stop.' % oa('P'), BODYJ))
A(P('That’s all there is to it. To see more, choose <b>Mouse Tutorial</b> from the '
    'Apple menu: it opens a document that shows each feature, which you can change as '
    'much as you like without harming the original.', BODYJ))

# ============================================================ reference
MENU_INTRO = {
    'Apple': ('m_apple', (0, 0, 300, 120)),
    'File': ('m_file', (0, 0, 300, 120)),
    'Edit': ('m_edit', (0, 0, 300, 120)),
    'Search': ('m_search', (0, 0, 300, 120)),
    'Format': ('m_format', (100, 0, 400, 128.5)),
    'Font': ('m_font', (100, 0, 400, 120)),
    'Size': ('m_size', (150, 0, 450, 120)),
    'Style': ('m_style', (260, 0, 560, 110)),
    'Windows': ('m_windows2', (260, 0, 560, 110)),
}


def menu_chapter(num, menu, intro):
    title = 'The %s Menu' % menu
    chapter(num, title, intro)
    name, crop = MENU_INTRO[menu]
    if os.path.exists(scrn(name)):
        A(shot(scrn(name), 'The %s menu' % menu, width=300, crop=crop))


from content import reference          # noqa: E402  (the reference chapters)
reference(globals())

# ============================================================ index
chapter('', 'Index')
index = SimpleIndex(style=ParagraphStyle('ix', fontName='Helvetica', fontSize=8.4, leading=10.4),
                    dot=' ', headers=True)
index.textStyle = ParagraphStyle('ix', fontName='Helvetica', fontSize=8.4, leading=10.4)
A(index)


# ============================================================ page templates
class Doc(BaseDocTemplate):
    def afterFlowable(self, f):
        if isinstance(f, Bookmark) and hasattr(f, 'toc'):
            level, text, key = f.toc
            self.notify('TOCEntry', (level, text, self.page, key))
            if hasattr(f, 'chapter'):
                _chapter[0] = f.chapter


def page_deco(c, doc):
    n = doc.page
    c.saveState()
    odd = n % 2 == 1
    bx = W - 62 if odd else 38
    c.setFillColor(YEL)
    c.rect(bx, 34, 24, 18, stroke=0, fill=1)
    c.setFillColor(black)
    c.setFont('Helvetica-Bold', 9)
    c.drawCentredString(bx + 12, 39.5, str(n))
    c.setFont('Helvetica-Oblique', 7.2)
    label = 'RetroWrite II   ·   ' + _chapter[0] if _chapter[0] else 'RetroWrite II'
    if odd:
        c.drawRightString(bx - 8, 40, label)
    else:
        c.drawString(bx + 32, 40, label)
    c.setStrokeColor(GREY)
    c.setLineWidth(0.5)
    c.line(X0, 60, X1, 60)
    c.restoreState()


def cover(c, doc):
    c.saveState()
    c.setFillColor(white)
    c.rect(0, 0, W, H, stroke=0, fill=1)
    c.setFillColor(YEL)
    c.rect(0, H - 250, W, 190, stroke=0, fill=1)
    c.setFillColor(black)
    head_text(c, 60, H - 150, 'RetroWrite II', 64)
    head_text(c, 64, H - 190, 'Word Processing for the Apple IIe', 20)
    head_text(c, 64, H - 225, "User's Manual", 26, font='HeadR', scale=86)
    # the screen
    from reportlab.lib.utils import ImageReader
    from PIL import Image as PI
    im = PI.open(scrn('letter'))
    w = 360
    h = w * im.height / im.width
    x = (W - w) / 2
    y = H - 290 - h
    c.setFillColor(black)
    c.roundRect(x - 14, y - 14, w + 28, h + 28, 10, stroke=0, fill=1)
    c.drawImage(scrn('letter'), x, y, w, h)
    # logo and version
    logo = os.path.join(ART, '6502_retro.png')
    if os.path.exists(logo):
        li = PI.open(logo)
        lw = 120
        lh = lw * li.height / li.width
        c.drawImage(logo, 60, 150, lw, lh, mask='auto')
    head_text(c, 200, 196, 'Version 2.1', 22)
    c.setFont('Helvetica', 11)
    c.drawString(200, 176, '6502 Retro Computing  ·  2026')
    c.setFont('Helvetica-Oblique', 8.5)
    c.drawString(200, 160, 'For the enhanced Apple IIe with 128K, a mouse and a hard disk')
    c.setStrokeColor(black)
    c.setLineWidth(0.8)
    c.rect(60, 60, W - 120, 56, stroke=1, fill=0)
    c.setFont('Helvetica-Bold', 8)
    c.drawString(70, 102, 'An enthusiast program and manual, 2026')
    c.setFont('Helvetica', 7.6)
    c.drawString(70, 89, 'RetroWrite II is an independent word processor by 6502 Retro Computing, written in 2026 for Apple II')
    c.drawString(70, 78, 'enthusiasts. It is not a product of Apple, nor of the publishers of MultiScribe, MouseWrite or MacWrite.')
    c.drawString(70, 67, 'This manual accompanies RetroWrite II version 2.1.')
    c.restoreState()


def build(out):
    doc = Doc(out, pagesize=(W, H), title='RetroWrite II User’s Manual',
              author='6502 Retro Computing', subject='RetroWrite II 2.1 for the Apple IIe')
    frame = Frame(X0, 66, COLW, H - 66 - 58, leftPadding=0, rightPadding=0,
                  topPadding=0, bottomPadding=0)
    doc.addPageTemplates([
        PageTemplate('cover', frames=[frame], onPage=cover),
        PageTemplate('plain', frames=[frame]),
        PageTemplate('body', frames=[frame], onPageEnd=page_deco),
    ])
    doc.multiBuild(story, canvasmaker=index.getCanvasMaker())


if __name__ == '__main__':
    build(sys.argv[1] if len(sys.argv) > 1 else 'RetroWrite_II_Manual.pdf')
