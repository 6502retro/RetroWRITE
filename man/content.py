"""The reference chapters and appendices of the RetroWrite II manual."""
from style import *
from reportlab.platypus import Paragraph, Spacer, PageBreak, KeepTogether


def reference(g):
    A, P, entry, sub, chapter, shot, scrn = (g['A'], g['P'], g['entry'], g['sub'],
                                             g['chapter'], g['shot'], g['scrn'])
    ix = g['ix']
    menu_chapter = g['menu_chapter']
    D, T, E, N, SEE = 'Definition', 'What to do', 'Example', 'NOTE', 'See'

    # ------------------------------------------------------------- 3 Apple
    menu_chapter('3', 'Apple', [
        'The Apple menu, at the left end of the menu bar, holds information about '
        'RetroWrite II, the Mouse Tutorial, your Startup Preferences and the desk '
        'accessories: small programs you can use without leaving your work.'])

    entry('About RetroWrite II', 'Apple', None,
          (D, 'Shows the version of RetroWrite II and who made it.'),
          (T, *steps('Choose About RetroWrite II... from the Apple menu.', 'Click OK (or press Return).')),
          (SEE, 'Keyboard Help'))
    A(shot(scrn('about'), 'About RetroWrite II', width=280, crop=(80, 40, 480, 160)))

    entry('Keyboard Help', 'Apple', '@-?',
          (D, 'Shows a short list of the keyboard commands.'),
          (T, *steps('Choose Keyboard Help from the Apple menu, or press %s.' % oa('?'),
                     'Click OK when you have read it.')),
          (N, 'You don’t need to hold Shift: %s works too. Appendix A has the full list.' % oa('/')),
          (SEE, 'Appendix A, Keyboard Summary'))

    entry('Mouse Tutorial', 'Apple', None,
          (D, 'Opens the TUTORIAL document: a guided tour of RetroWrite II that shows each '
              'feature in a real document.'),
          (T, *steps('Choose Mouse Tutorial from the Apple menu.',
                     'Read it, scroll with the scroll bar, and try things out in it.')),
          (N, 'The tutorial opens as an untitled copy, so you can change it freely; the '
              'original file is never changed. If you save it, give it a new name.'),
          (SEE, 'Chapter 2, A Quick Tour'))

    entry('Startup Preferences', 'Apple', None,
          (D, 'Sets how RetroWrite II starts: whether the ruler shows, the font and size of '
              'new documents, the folder the Open and Save dialogs start in, and the printer '
              'settings from Page Setup. They are kept in the file %s.' % scr('RW2.PREFS')),
          (T, *steps('Choose Startup Preferences... from the Apple menu.',
                     'Click the choices you want: <b>Show the ruler</b> Yes or No; <b>New documents</b> '
                     'Sans, Serif or Mono; <b>Size</b> 9, 18 or 27 Point.',
                     'Click <b>Save</b> to keep them, or Cancel.')),
          (N, '<b>Documents</b> shows the folder the Open and Save dialogs will start in: the '
              'last folder you used. Clicking Save also keeps the printer, slot and print '
              'quality you chose in Page Setup, so set those first.'),
          (SEE, 'Page Setup; Show Ruler'))
    A(shot(scrn('prefs'), 'Startup Preferences', width=290, crop=(40, 30, 520, 180)))

    entry('Load DOS 3.3 File', 'Apple', None,
          (D, 'Reads a text file from a DOS 3.3 disk in a 5.25-inch drive and puts it into '
              'your document at the caret. Use it for letters and notes you wrote years ago '
              'with DOS 3.3 programs or Applesoft.'),
          (T, *steps('Put the DOS 3.3 disk in drive 1 or 2 of the disk card in slot 6.',
                     'Click where the text should go in your document.',
                     'Choose Load DOS 3.3 File... from the Apple menu.',
                     'Click <b>S6 D1</b> or <b>S6 D2</b> for the drive.',
                     'The text files on the disk are listed. Click one and click it again, '
                     'or use the arrow keys and press Return. Esc cancels.')),
          (N, 'Only text files (DOS type T) are listed, up to 24. Lines end where the original '
              'had Return; the text takes the style of the text at the caret.'),
          (SEE, 'Open'), index=['Load DOS 3.3 File', 'DOS 3.3 disks'])
    A(shot(scrn('dos2'), 'Choosing a text file from a DOS 3.3 disk', width=280, crop=(80, 20, 480, 120)))

    entry('Communications', 'Apple', None,
          (D, 'Turns RetroWrite II into a simple terminal for a Super Serial Card, so you '
              'can talk to another computer or a modem. The text you receive can be kept in '
              'your document.'),
          (T, *steps('Choose Communications... from the Apple menu.',
                     'Click the <b>Card slot</b> of your Super Serial Card and the <b>Speed</b> '
                     '(300, 1200, 2400, 9600 or 19200 baud).',
                     'Choose <b>Keep the text</b> Yes if what you receive should go into your '
                     'document at the caret.',
                     'Click <b>Connect</b>. What you type is sent; what comes in is shown.',
                     'Press %s (period) to end.' % oa('.'))),
          (N, 'The card is set to 8 data bits, no parity, 1 stop bit. If no Super Serial Card '
              'is in the slot you chose, RetroWrite II tells you so.'),
          (SEE, 'Startup Preferences'), index=['Communications', 'Super Serial Card', 'terminal'])
    A(shot(scrn('comm'), 'Communications settings', width=290, crop=(40, 30, 520, 160)))

    entry('BASIC.SYSTEM', 'Apple', None,
          (D, 'Leaves RetroWrite II for Applesoft BASIC (BASIC.SYSTEM). When you type '
              '%s in BASIC you come straight back to RetroWrite II.' % scr('BYE')),
          (T, *steps('Choose BASIC.SYSTEM from the Apple menu.',
                     'If a document has unsaved changes, you are asked whether to save it.',
                     'Click <b>Go</b>. The BASIC prompt %s appears.' % scr(']'),
                     'When you are done, type %s and press Return.' % scr('BYE'))),
          (N, 'RetroWrite II starts again with an empty document; open your document again '
              'with Open. BASIC.SYSTEM must be in the RETROWRITE folder or at the top level '
              'of the disk.'),
          (SEE, 'Quit'), index=['BASIC.SYSTEM', 'BYE'])

    # ------------------------------------------------------------- 4 File
    menu_chapter('4', 'File', [
        'The File menu creates, opens, saves, prints and closes documents. Its first line '
        'shows the name of the document on the screen.'])

    entry('New', 'File', '@-N',
          (D, 'Starts a new, empty document called Untitled.'),
          (T, *steps('Choose New from the File menu, or press %s.' % oa('N'))),
          (N, 'If the document on the screen has text in it, the new document opens in a '
              'new window and the old one stays open in the Windows menu (up to four '
              'documents). If all four windows are in use, you are asked whether to save the '
              'changes to the one on the screen, which the new document then replaces.'),
          (SEE, 'The Windows Menu; Close'))

    entry('Open', 'File', '@-O',
          (D, 'Opens a RetroWrite II document, an ODT document (LibreOffice, OpenOffice or '
              'Word) or a text file from a disk.'),
          (T, *steps('Choose Open... from the File menu, or press %s.' % oa('O'),
                     'The dialog lists the folders (%s) and documents (%s) in the current '
                     'folder. Click a document and click <b>Open</b>, or double-click it.'
                     % ('□ folder icons', 'page icons'),
                     'To open a folder, double-click it. Click <b>Up</b> to go to the folder '
                     'that holds this one, and <b>Disks</b> to see all the disks.')),
          (E, 'To open the sample letter, press %s, type %s (the list jumps to the first '
              'name starting with L) and press Return.' % (oa('O'), scr('L'))),
          (N, 'Keyboard: up and down arrows choose, right arrow or Return opens, left arrow '
              'goes up a folder, Esc cancels. Only folders, RetroWrite II documents, ODT '
              'documents (names ending in .ODT) and text files are listed, up to 48 per '
              'folder. A text file opens as plain text in Sans 9 point. An ODT document shows '
              'a bar while it is unpacked; see “ODT documents” below.'),
          (SEE, 'New; Save; Startup Preferences'), index=['Open', 'file dialog', 'text files'])
    A(shot(scrn('open'), 'The Open dialog', width=290, crop=(60, 20, 500, 180)))

    entry('Close', 'File', None,
          (D, 'Closes the document on the screen.'),
          (T, *steps('Choose Close from the File menu.',
                     'If there are changes, click <b>Save</b>, <b>Don’t Save</b> or '
                     '<b>Cancel</b>.')),
          (N, 'If other documents are open, the next one comes on the screen. If it was the '
              'only one, you get an empty Untitled document.'),
          (SEE, 'The Windows Menu; Quit'))
    A(shot(scrn('confirm'), 'Asked when a changed document would be closed', width=270,
           crop=(90, 40, 480, 120)))

    entry('Save', 'File', '@-S',
          (D, 'Saves the document on the screen under its name.'),
          (T, *steps('Choose Save from the File menu, or press %s.' % oa('S'))),
          (N, 'If the document has no name yet, the Save As dialog appears. A document that '
              'came from an ODT document is saved as ODT again. A document that '
              'came from a text file is saved as a text file again; if you have added fonts '
              'or styles to it, RetroWrite II asks whether to save it as a RetroWrite '
              'document (which keeps them) or as text. Save often!'),
          (SEE, 'Save As; Save As Text'))

    entry('Save As', 'File', '@-A',
          (D, 'Saves the document under a new name or in another folder, normally as an ODT '
              'document that LibreOffice, OpenOffice and Word can open. It keeps the fonts, '
              'styles, ruler, header and footer.'),
          (T, *steps('Choose Save As... from the File menu, or press %s.' % oa('A'),
                     'Open the folder you want, as in the Open dialog.',
                     'Type the name in the <b>Name</b> box (Delete erases). Clicking a '
                     'document in the list copies its name. A name without a period gets '
                     '%s added.' % scr('.ODT'),
                     'Click <b>Save</b> or press Return.')),
          (N, 'Names have up to 15 letters, digits and periods and start with a letter, as '
              'ProDOS requires. If a file with that name exists, you are asked whether to '
              'replace it. To save in RetroWrite II’s own format instead, give a name with any '
              'other ending, such as %s; these are ProDOS type $50 with aux type $5257.'
              % scr('LETTER.RW')),
          (SEE, 'Save; Save As Text'), index=['Save As', 'file names'])
    A(shot(scrn('saveas'), 'The Save As dialog', width=290, crop=(60, 26, 500, 192)))

    entry('Save As Text', 'File', None,
          (D, 'Saves the document as a plain ProDOS text file (TXT), which other programs '
              'such as AppleWorks, BASIC and Apple II DeskTop can read.'),
          (T, *steps('Choose Save As Text... from the File menu.',
                     'Choose the folder and type a name, then click Save.')),
          (N, 'A text file can’t keep fonts, sizes, styles, the ruler or page breaks; '
              'RetroWrite II warns you if the document has any. Each paragraph becomes one '
              'line ending with Return.'),
          (SEE, 'Save As'), index=['Save As Text', 'text files'])

    sub('ODT documents')
    A(P(ix('ODT documents', 'LibreOffice', 'Word') +
        'ODT (OpenDocument Text) is the standard document format of LibreOffice and '
        'OpenOffice; Microsoft Word, Google Docs and Apple Pages open it too. RetroWrite II '
        'saves and opens ODT documents itself, so you can carry your work between the '
        'Apple IIe and a modern computer: copy the file with a disk-image tool such as '
        'CiderPress II or AppleCommander, or a shared hard disk, and double-click it there. '
        'Going the other way, save from your word processor as ODT, with a name of up to 15 '
        'letters, digits and periods ending in %s.' % scr('.ODT'), BODYJ))
    A(table([['RetroWrite II', 'In the ODT document'],
             ['Sans, Serif, Mono', 'Liberation Sans, Liberation Serif, Liberation Mono '
              '(other fonts open as the nearest of the three)'],
             ['9, 18 and 27 point', '12, 24 and 36 point, which keeps lines about as long '
              'as RetroWrite II prints them (opening: up to 17 point is 9, up to 29 is 18)'],
             ['Bold, italic, underline, superscript, subscript', 'the same'],
             ['Alignment, line spacing, page breaks', 'the same'],
             ['Ruler margins and tab stops', 'paragraph indents and tab stops'],
             ['Header and footer, # for the page number', 'page header and footer with a '
              'page-number field'],
             ['Letters', 'plain ASCII; when opening, accented letters lose their accents and '
              'curly quotes and dashes become straight ones']],
            [150, COLW - 150]))
    A(Spacer(1, 6))
    A(P('When RetroWrite II opens an ODT document it leaves out pictures, '
        'footnotes and comments (the text in tables is kept, one paragraph per cell), and '
        'list items start with a dash. Opening takes time on a 1 MHz Apple IIe: about half '
        'a minute for a short letter saved by LibreOffice, and less for documents RetroWrite '
        'II saved itself, which are not compressed. Two work files, %s and %s, are made in '
        'the RETROWRITE folder while a document is opened or saved, so the disk needs some '
        'free space.' % (scr('RW2.XML1'), scr('RW2.XML2')), NOTE))
    A(shot(scrn('odtopen'), 'Opening an ODT document', width=250, crop=(80, 50, 480, 130)))
    A(shot(scrn('odtnews'), 'A document from LibreOffice, opened in RetroWrite II', width=290))

    entry('Page Setup', 'File', None,
          (D, 'Chooses your printer and how to print, and sets the header, footer and page '
              'numbers of the document.'),
          (T, *steps('Choose Page Setup... from the File menu.',
                     'Click your <b>Printer</b>: ImageWriter II, Epson, or Plain text (any '
                     'printer, no styles).',
                     'Click the <b>Slot</b> of the printer card.',
                     'Choose <b>Print</b>: <b>As on screen</b> prints in graphics, exactly as '
                     'you see it; <b>Faster (printer’s font)</b> uses the printer’s own '
                     'letters, with bold, underline, superscript and subscript.',
                     '<b>Page numbers</b> Yes prints %s at the foot of each page when there is '
                     'no footer.' % scr('- 1 -'),
                     'Click the <b>Header</b> or <b>Footer</b> box to type a line printed at the '
                     'top or bottom of every page. A %s in it prints the page number.' % scr('#'),
                     'Click <b>OK</b>.')),
          (E, 'A footer of %s prints %s, %s ...' % (scr('Club newsletter - page #'),
                                                    scr('Club newsletter - page 1'), scr('page 2'))),
          (N, 'The header and footer are saved with the document. The printer, slot and print '
              'choices last until you quit; to keep them, click Save in Startup Preferences.'),
          (SEE, 'Print; Startup Preferences; Insert Page Break'),
          index=['Page Setup', 'headers and footers', 'page numbers', 'printers'])
    A(shot(scrn('pagesetup'), 'Page Setup', width=300, crop=(40, 20, 520, 192)))

    entry('Print', 'File', '@-P',
          (D, 'Prints the document on the screen.'),
          (T, *steps('Turn on the printer and make sure it is selected (on line).',
                     'Choose Print... from the File menu, or press %s.' % oa('P'),
                     'RetroWrite II names the printer and slot; click <b>Print</b>. Click '
                     '<b>Setup...</b> to change them first.',
                     'A box shows the page being printed. Press Esc to stop.')),
          (N, 'Pages are 11 inches long with the text 6.4 inches wide, like the ruler. '
              'Printing <b>As on screen</b> sends the page as graphics, which takes longer than '
              '<b>Faster</b>. If no printer card is in the slot, RetroWrite II says so.'),
          (SEE, 'Page Setup; Print Several'), index=['Print', 'printing'])
    A(shot(scrn('print'), 'Before printing', width=270, crop=(90, 40, 480, 120)))

    entry('Print Several', 'File', None,
          (D, 'Prints a series of documents, one after the other, without opening each one '
              'yourself.'),
          (T, *steps('Choose Print Several... from the File menu.',
                     'In the dialog, open the first document to print; it is printed.',
                     'The dialog appears again: choose the next one, and so on.',
                     'Click <b>Cancel</b> when you are done. Your own document comes back on '
                     'the screen.')),
          (N, 'All the documents are printed with the printer settings in Page Setup and '
              'each document’s own header and footer.'),
          (SEE, 'Print'))

    entry('Quit', 'File', '@-Q',
          (D, 'Leaves RetroWrite II.'),
          (T, *steps('Choose Quit from the File menu, or press %s.' % oa('Q'),
                     'For each open document with unsaved changes, click Save or Don’t '
                     'Save (Cancel stops quitting).')),
          (N, 'You return to the program that started RetroWrite II, such as Apple II DeskTop.'),
          (SEE, 'Close'), index=['Quit', 'quitting'])

    # ------------------------------------------------------------- 5 Edit
    menu_chapter('5', 'Edit', [
        'The Edit menu moves and copies text through the clipboard, selects the whole '
        'document and types glossary entries for you.'])

    entry('Cut', 'Edit', '@-X',
          (D, 'Removes the selected text and puts it on the clipboard, ready to paste.'),
          (T, *steps('Select the text.', 'Choose Cut from the Edit menu, or press %s.' % oa('X'))),
          (N, 'The clipboard holds one piece of text at a time, with its fonts and styles. It '
              'is kept in the file %s, so it is still there after you quit and start '
              'RetroWrite II again, or open another document.' % scr('RW2.CLIPBOARD')),
          (SEE, 'Copy; Paste; Clear'), index=['Cut', 'clipboard'])

    entry('Copy', 'Edit', '@-C',
          (D, 'Puts a copy of the selected text on the clipboard, leaving the document as it is.'),
          (T, *steps('Select the text.', 'Choose Copy from the Edit menu, or press %s.' % oa('C'))),
          (SEE, 'Cut; Paste'))

    entry('Paste', 'Edit', '@-V',
          (D, 'Puts the text on the clipboard into the document at the caret, or in place of '
              'the selected text.'),
          (T, *steps('Click where the text should go, or select text to replace.',
                     'Choose Paste from the Edit menu, or press %s.' % oa('V'))),
          (E, 'To move a paragraph: select it, press %s, click where it should go and press %s.'
              % (oa('X'), oa('V'))),
          (N, 'You can paste the same text as many times as you like.'),
          (SEE, 'Cut; Copy'))

    entry('Clear', 'Edit', None,
          (D, 'Removes the selected text without putting it on the clipboard.'),
          (T, *steps('Select the text.', 'Choose Clear from the Edit menu, or press Delete.')),
          (SEE, 'Cut'))

    entry('Select All', 'Edit', '@-W',
          (D, 'Selects the whole document.'),
          (T, *steps('Choose Select All from the Edit menu, or press %s.' % oa('W'))),
          (E, 'To change the font of the whole document: press %s, then choose the font.'
              % oa('W')),
          (SEE, 'Using the mouse (chapter 1)'), index=['Select All', 'selecting text'])

    entry('Glossary', 'Edit', 'Control-G',
          (D, 'Types a stored word or phrase for you. You give each phrase a one-key name in '
              'the glossary; Control-G and that key put it into your document at the caret.'),
          (T, *steps('Hold down Control and press G (or choose Glossary from the Edit menu).',
                     'A box asks for the entry’s key. Press it. Esc cancels.')),
          (E, 'With the entry %s in the glossary, Control-G then %s types %s.'
              % (scr('A=Apple IIe'), scr('A'), scr('Apple IIe'))),
          (N, 'Press %s instead of a key to see the list of keys. Letters can be typed in capitals '
              'or small letters. The text takes the current typing style. The glossary that comes '
              'with RetroWrite II has A, R, D, Y and S.' % scr('?')),
          (SEE, 'Edit Glossary'), index=['Glossary', 'Control-G'])
    A(shot(scrn('gloss'), 'Waiting for the glossary key', width=270, crop=(100, 60, 460, 110)))

    entry('Edit Glossary', 'Edit', None,
          (D, 'Opens the glossary file so you can add, change or remove entries.'),
          (T, *steps('Choose Edit Glossary from the Edit menu. The %s file opens as a text document.'
                     % scr('GLOSSARY'),
                     'Each line is one entry: the key, an equal sign and the text, such as %s.'
                     % scr('Y=Yours sincerely,'),
                     'Change the lines, then choose Save (%s).' % oa('S'))),
          (N, 'Each entry fits on one line, up to about 190 characters. A key can be any '
              'letter, digit or sign. The file is in the RETROWRITE folder; if it doesn’t '
              'exist, Edit Glossary starts a new one.'),
          (SEE, 'Glossary'))

    # ------------------------------------------------------------- 6 Search
    menu_chapter('6', 'Search', [
        'The Search menu finds and replaces words and checks your spelling.'])

    entry('Find', 'Search', '@-F',
          (D, 'Finds a word or phrase in the document and selects it.'),
          (T, *steps('Choose Find... from the Search menu, or press %s.' % oa('F'),
                     'Type what to find (up to 30 characters) and click OK or press Return. The box '
                     'shows what you looked for last time; press Delete to erase it.')),
          (E, 'Find %s finds %s, %s and %s.' % (scr('the'), scr('The'), scr('the'), scr('weather'))),
          (N, 'Find ignores capitals and fonts and also finds the letters inside longer words. '
              'It searches from the selection to the end, then from the top. If the text is '
              'not in the document, RetroWrite II says Not found.'),
          (SEE, 'Find Next; Replace'), index=['Find', 'searching'])
    A(shot(scrn('find'), 'Find', width=270, crop=(120, 50, 420, 140)))

    entry('Find Next', 'Search', '@-G',
          (D, 'Finds the next place with the same text you last looked for.'),
          (T, *steps('Choose Find Next from the Search menu, or press %s.' % oa('G'))),
          (SEE, 'Find'))

    entry('Replace', 'Search', '@-R',
          (D, 'Finds a word or phrase and replaces it with another, one by one or all at once.'),
          (T, *steps('Choose Replace... from the Search menu, or press %s.' % oa('R'),
                     'Type what to find and press Return, then what to put in its place and '
                     'press Return.',
                     'Each time the text is found, it is selected and you are asked: '
                     '<b>Replace</b> replaces it and goes on, <b>All</b> replaces it everywhere '
                     'in the document, <b>Stop</b> stops.')),
          (N, 'At the end RetroWrite II tells you how many times it replaced the text. The '
              'replacement takes the style of the text it replaces.'),
          (SEE, 'Find'), index=['Replace', 'searching'])
    A(shot(scrn('replace'), 'Replacing one at a time', width=270, crop=(90, 40, 480, 120)))

    entry('Check Spelling', 'Search', '@-K',
          (D, 'Checks every word against the 460K dictionary SPELL.DICT and your own user '
              'dictionary, and shows you the words it doesn’t know.'),
          (T, *steps('Click where checking should start (at the end of the document, it '
                     'checks everything).',
                     'Choose Check Spelling from the Search menu, or press %s.' % oa('K'),
                     'For each unknown word, which is selected in your text, click:',
                     '<b>Skip</b> to leave it; <b>Replace</b> to type the correct word; '
                     '<b>Ignore All</b> to skip it everywhere until you quit; <b>Learn</b> to add '
                     'it to your user dictionary; <b>Suggest</b> (or press G) to see likely '
                     'words; <b>Stop</b> (or Esc) to stop.',
                     'To use a suggestion, click it or press its number, 1 to 9.')),
          (N, 'A replacement keeps the capitals of the original word. When the end is reached, '
              'you can go on from the top. Words with digits are skipped.'),
          (SEE, 'Edit User Dictionary'), index=['Check Spelling', 'spelling checker', 'SPELL.DICT'])
    A(shot(scrn('spellsugg'), 'An unknown word, with suggestions', width=300, crop=(0, 100, 560, 192)))

    entry('Edit User Dictionary', 'Search', None,
          (D, 'Opens your user dictionary, USER.DICT, the list of words you taught the '
              'spelling checker with Learn.'),
          (T, *steps('Choose Edit User Dictionary from the Search menu.',
                     'The words appear one to a line. Change, add or remove words.',
                     'Choose Save (%s).' % oa('S'))),
          (N, 'Words are kept in small letters. Names and special words you use often are '
              'good to add here.'),
          (SEE, 'Check Spelling'), index=['Edit User Dictionary', 'USER.DICT'])

    # ------------------------------------------------------------- 7 Format
    menu_chapter('7', 'Format', [
        'The Format menu and the ruler shape your paragraphs: margins, tab stops, '
        'alignment, line spacing and page breaks. Margins and tab stops belong to the '
        'whole document. Alignment and spacing belong to each paragraph: they apply to the '
        'paragraph with the caret, or to every paragraph in the selection.'])

    entry('Show Ruler', 'Format', '@-H',
          (D, 'Shows or hides the ruler at the top of the window.'),
          (T, *steps('Choose Show Ruler from the Format menu, or press %s. A check mark shows '
                     'when the ruler is on; choose it again to hide the ruler.' % oa('H'))),
          (SEE, 'The ruler (below); Startup Preferences'), index=['Show Ruler', 'ruler'])
    A(shot(scrn('letter'), 'The ruler', width=330, crop=(0, 10, 560, 40), callouts=[
        (14, 23, 'left margin', 'L'), (130, 18, 'scale (inches)', 'L'),
        (20, 32, 'alignment', 'L'), (100, 32, 'spacing', 'L'),
        (60, 22, 'tab stops', 'R'), (522, 23, 'right margin', 'R')]))

    g['sub']('Using the ruler')
    A(P(ix('ruler', 'margins', 'tab stops') + 'The scale is marked in inches, from 0 to 6.4. '
        'The two triangles under it are the left and right margins; the small marks are '
        'tab stops.', BODYJ))
    for t in bullets(
            '<b>Margins:</b> drag a triangle left or right. The margins are at least ½ '
            'inch apart.',
            '<b>A new tab stop:</b> click the scale where you want it. You can have up to 8.',
            '<b>Moving a tab stop:</b> drag it along the scale.',
            '<b>Removing a tab stop:</b> drag it down off the ruler.',
            '<b>Alignment:</b> click one of the four icons: left, centered, right or justified.',
            '<b>Line spacing:</b> click one of the three icons: single, 1½ or double.'):
        A(t)
    A(P('Positions snap to 1/16 inch. Pressing Tab moves to the next tab stop; past the '
        'last one, or if there are none, it moves to the next ½ inch. The ruler is '
        'saved with the document.', NOTE))

    entry('Align Left', 'Format', '@-L',
          (D, 'Lines up the paragraph’s lines at the left margin, with a ragged right '
              'edge. This is how new paragraphs start.'),
          (T, *steps('Click in the paragraph (or select several).',
                     'Choose Align Left from the Format menu, press %s, or click the first '
                     'alignment icon in the ruler.' % oa('L'))),
          (SEE, 'Center; Align Right; Justify'), index=['Align Left', 'alignment'])

    entry('Center', 'Format', '@-E',
          (D, 'Centers each line of the paragraph between the margins. Use it for titles '
              'and headings.'),
          (T, *steps('Click in the paragraph.', 'Choose Center from the Format menu or press %s.' % oa('E'))),
          (SEE, 'Align Left'), index=['Center', 'alignment'])

    entry('Align Right', 'Format', None,
          (D, 'Lines up the paragraph’s lines at the right margin, for dates and '
              'addresses at the right side of a letter.'),
          (T, *steps('Click in the paragraph.', 'Choose Align Right from the Format menu.')),
          (SEE, 'Align Left'), index=['Align Right', 'alignment'])

    entry('Justify', 'Format', '@-J',
          (D, 'Spreads the words of each line so both edges of the paragraph are straight, '
              'as in a book. The last line of the paragraph stays at the left.'),
          (T, *steps('Click in the paragraph.', 'Choose Justify from the Format menu or press %s.' % oa('J'))),
          (SEE, 'Align Left'), index=['Justify', 'alignment'])

    entry('Single, 1½ and Double Space', 'Format', None,
          (D, 'Sets the space between the lines of the paragraph.'),
          (T, *steps('Click in the paragraph or select several.',
                     'Choose Single Space, 1 1/2 Space or Double Space from the Format menu, '
                     'or click a spacing icon in the ruler.')),
          (N, 'Double spacing is handy for drafts that others will mark up on paper.'),
          (SEE, 'Using the ruler'), index=['line spacing', 'Double Space'])

    entry('Insert Page Break', 'Format', None,
          (D, 'Makes the text after the caret start on a new page when you print.'),
          (T, *steps('Click where the new page should start.',
                     'Choose Insert Page Break from the Format menu.')),
          (N, 'A page break shows as a dotted line across the window. To remove it, click just '
              'after it and press Delete. Without page breaks, RetroWrite II starts a new page '
              'when one is full.'),
          (SEE, 'Print'), index=['Insert Page Break', 'page breaks'])

    # ------------------------------------------------------------- 8 Font
    menu_chapter('8', 'Font', [
        'RetroWrite II has three fonts, all designed for the Apple IIe’s double '
        'hi-res screen and printed as you see them.'])
    A(table([['Font', 'Looks like', 'Good for'],
             ['Sans', 'clean letters without serifs', 'most writing, headings'],
             ['Serif', 'letters with small serifs, like a book', 'letters, reports, titles'],
             ['Mono', 'every letter the same width, like a typewriter',
              'tables, lists that must line up, program listings']], [60, 180, COLW - 240]))
    A(Spacer(1, 8))
    entry('Sans, Serif, Mono', 'Font', None,
          (D, 'Changes the font of the selected text, or of the text you type next.'),
          (T, *steps('Select the text (or, to choose the font for what you type next, just '
                     'put the caret where you will type).',
                     'Choose Sans, Serif or Mono from the Font menu.')),
          (N, 'The check mark shows the font at the start of the selection. The font for new '
              'documents is set in Startup Preferences.'),
          (SEE, 'The Size Menu; The Style Menu; Startup Preferences'), index=['fonts', 'Sans', 'Serif', 'Mono'])

    # ------------------------------------------------------------- 9 Size
    menu_chapter('9', 'Size', [
        'Each font comes in three sizes. 18 and 27 point are the 9 point letters drawn two '
        'and three times as large, for headings and titles.'])
    entry('9, 18 and 27 Point', 'Size', None,
          (D, 'Changes the size of the selected text, or of the text you type next.'),
          (T, *steps('Select the text.', 'Choose 9 Point, 18 Point or 27 Point from the Size menu.')),
          (N, 'A line is as tall as its largest letters. With Faster printing, 18 and 27 point '
              'print as wide printer letters.'),
          (SEE, 'The Font Menu'), index=['sizes', 'point sizes'])

    # ------------------------------------------------------------- 10 Style
    menu_chapter('10', 'Style', [
        'Styles change the look of letters in any font and size. You can combine bold, '
        'italic and underline.'])
    entry('Plain Text', 'Style', '@-T',
          (D, 'Removes bold, italic, underline, superscript and subscript from the selection.'),
          (T, *steps('Select the text.', 'Choose Plain Text from the Style menu or press %s.' % oa('T'))),
          (SEE, 'Bold, Italic, Underline'), index=['Plain Text', 'styles'])
    entry('Bold, Italic, Underline', 'Style', '@-B   @-I   @-U',
          (D, 'Makes letters heavier, slanted, or underlined. Each command turns its style on '
              'for the selection, or off if the selection already has it.'),
          (T, *steps('Select the text.', 'Choose Bold, Italic or Underline from the Style menu, '
                     'or press %s, %s or %s.' % (oa('B'), oa('I'), oa('U')))),
          (E, 'Press %s, type %s, press %s again, and type on: only the word is bold.'
              % (oa('B'), scr('important'), oa('B'))),
          (N, 'With nothing selected, the style applies to what you type next. Check marks '
              'show the styles of the selected text.'),
          (SEE, 'Plain Text'), index=['Bold', 'Italic', 'Underline', 'styles'])
    entry('Superscript, Subscript', 'Style', None,
          (D, 'Makes small letters raised above the line (superscript), as in E=mc² or '
              '2<super>nd</super>, or lowered below it (subscript), as in H<sub>2</sub>O.'),
          (T, *steps('Select the characters.', 'Choose Superscript or Subscript from the Style menu.',
                     'To go back to normal letters, choose Plain Text.')),
          (SEE, 'Plain Text'), index=['Superscript', 'Subscript'])

    # ------------------------------------------------------------- 11 Windows
    menu_chapter('11', 'Windows', [
        'RetroWrite II can have up to four documents open at once. One is on the screen; the '
        'others wait in the Windows menu. You can copy and paste between them.'])
    entry('Switching windows', 'Windows', None,
          (D, 'Brings another open document onto the screen.'),
          (T, *steps('Open the Windows menu: it lists the open documents, with a check mark '
                     'beside the one on the screen.',
                     'Choose the document you want.')),
          (N, 'New and Open use a new window when the document on the screen has something in '
              'it. While a document is not on the screen, it waits on the hard disk in a file '
              'named %s to %s in the RETROWRITE folder, with all its changes, so no extra '
              'memory card is needed.'
              % (scr('RW2.WIN0'), scr('RW2.WIN3'))),
          (SEE, 'New; Open; Close'), index=['windows', 'Windows menu'])

    # ------------------------------------------------------------- appendices
    appendices(g)


def appendices(g):
    A, P, chapter, shot, scrn, ix = g['A'], g['P'], g['chapter'], g['shot'], g['scrn'], g['ix']
    sub = g['sub']

    chapter('A', 'Keyboard Summary', [ix('keyboard') +
            'Hold %s (Open-Apple) or %s (Solid-Apple) and press the key shown.' % (oa(), sa())])
    rows = [['Keys', 'Command', 'Keys', 'Command']]
    left = [(oa('N'), 'New'), (oa('O'), 'Open'), (oa('S'), 'Save'), (oa('A'), 'Save As'),
            (oa('P'), 'Print'), (oa('Q'), 'Quit'), (oa('X'), 'Cut'), (oa('C'), 'Copy'),
            (oa('V'), 'Paste'), (oa('W'), 'Select All'), (oa('F'), 'Find'), (oa('G'), 'Find Next'),
            (oa('R'), 'Replace'), (oa('K'), 'Check Spelling')]
    right = [(oa('H'), 'Show Ruler'), (oa('L'), 'Align Left'), (oa('E'), 'Center'),
             (oa('J'), 'Justify'), (oa('T'), 'Plain Text'), (oa('B'), 'Bold'), (oa('I'), 'Italic'),
             (oa('U'), 'Underline'), (oa('?'), 'Keyboard Help'), ('Control-G', 'Glossary'),
             ('Esc', 'open the menus'), ('', ''), ('', ''), ('', '')]
    for (a, b), (c, d) in zip(left, right):
        rows.append([a, b, c, d])
    A(table(rows, [60, 140, 60, COLW - 260]))
    A(Spacer(1, 10))
    sub('Moving and editing', toc=False)
    A(table([['Keys', 'What they do'],
             ['← →', 'one character left or right'],
             ['↑ ↓', 'one line up or down'],
             [oa('←') + '  ' + oa('→'), 'one word left or right'],
             [oa('↑') + '  ' + oa('↓'), 'one screen up or down'],
             [oa('1') + '  ' + oa('9'), 'to the top or the end of the document'],
             ['Return', 'end the paragraph'],
             ['Tab', 'to the next tab stop'],
             ['Delete', 'erase the character before the caret, or the selection'],
             [oa('Delete'), 'erase the character after the caret']], [90, COLW - 90]))
    A(Spacer(1, 10))
    sub('Menus and dialogs', toc=False)
    A(table([['Keys', 'What they do'],
             ['Esc', 'open the menus (File first); in a menu: close it'],
             ['← → in a menu', 'the menu to the left or right'],
             ['↑ ↓ in a menu', 'choose a command'],
             ['Return', 'give the chosen command; in a dialog: the button with the heavy border'],
             ['a letter in a menu', 'the command that starts with that letter'],
             ['a letter in a dialog', 'the button that starts with that letter (not in a dialog with a box you type into)'],
             ['Esc in a dialog', 'Cancel (or Stop)']], [110, COLW - 110]))
    A(Spacer(1, 10))
    sub('Mouse', toc=False)
    A(table([['Action', 'What it does'],
             ['click', 'place the caret'],
             ['drag', 'select text'],
             ['double-click', 'select a word'],
             ['double-click and drag', 'select whole words'],
             [oa() + ' or ' + sa() + ' click', 'extend the selection'],
             ['click a menu title', 'open the menu (it stays open)']], [110, COLW - 110]))

    chapter('B', 'Files and Folders', [
        'Everything RetroWrite II uses is in one folder, normally %s. Keep these files '
        'together; RetroWrite II finds the folder by itself.' % scr('/DESKTOP/RETROWRITE')])
    A(table([['File', 'What it is'],
             [scr('RW2.SYSTEM'), 'the program you start; shows the welcome picture'],
             [scr('RW2.MAIN'), 'the main part of the program'],
             [scr('RW2.LC') + ', ' + scr('RW2.FONTS'), 'more of the program, and the fonts'],
             [scr('RW2.OVL1') + ' - ' + scr('RW2.OVL9') + ', ' + scr('RW2.OVLA') + ', ' + scr('RW2.OVLB'),
              'parts read from the disk when needed: files, spelling, editing, menus, printing, '
              'desk accessories, dialogs, and saving and opening ODT documents'],
             [scr('RW2.SPLASH'), 'the welcome picture'],
             [scr('SPELL.DICT'), 'the dictionary (460K)'],
             [scr('TUTORIAL'), 'the Mouse Tutorial document'],
             [scr('GLOSSARY'), 'your glossary (a text file)'],
             [scr('BASIC.SYSTEM'), 'for Apple menu → BASIC.SYSTEM (optional)']],
            [130, COLW - 130]))
    A(Spacer(1, 8))
    A(P('These files are made as you work:', BODY))
    A(table([['File', 'What it is'],
             [scr('USER.DICT'), 'your own words for the spelling checker (Learn)'],
             [scr('RW2.PREFS'), 'your Startup Preferences'],
             [scr('RW2.CLIPBOARD'), 'the clipboard'],
             [scr('RW2.WIN0') + ' - ' + scr('RW2.WIN3'), 'documents waiting in other windows'],
             [scr('RW2.QUIT'), 'only while you are in BASIC.SYSTEM'],
             [scr('RW2.XML1') + ', ' + scr('RW2.XML2'), 'work files, only while an ODT document '
              'is opened or saved']], [130, COLW - 130]))
    A(Spacer(1, 8))
    A(P(ix('documents folder') + 'Your documents can be anywhere. The disk that comes with '
        'RetroWrite II has a %s folder, where the Open and Save dialogs start the first '
        'time.' % scr('DOCUMENTS'), BODYJ))

    chapter('C', 'Limits and Memory', [ix('limits', 'memory') +
            'RetroWrite II runs in an Apple IIe with 128K. It keeps your document in the '
            'auxiliary 64K and reads the less-used parts of the program from the hard disk.'])
    A(table([['', 'Limit'],
             ['Length of a document', '30,460 characters (about 15 typed pages)'],
             ['Open documents', '4 (in the Windows menu)'],
             ['Width of the text', '6.4 inches; margins at least ½ inch apart'],
             ['Tab stops', '8 (then every ½ inch)'],
             ['Fonts and sizes', '3 fonts, 3 sizes, bold, italic, underline, superscript, subscript'],
             ['Header and footer', '39 characters each'],
             ['Find and Replace text', '30 characters'],
             ['Glossary entry', 'about 190 characters'],
             ['File names', '15 characters (ProDOS)'],
             ['Files listed in a folder', '48'],
             ['ODT documents', 'up to 30,460 characters of text, as above; opening a long one '
              'takes a minute or more at 1 MHz'],
             ['Undo', 'there is no Undo: save often, and save copies of important work']],
            [140, COLW - 140]))
    A(Spacer(1, 8))
    A(P('<b>Speed.</b> On a 1 MHz Apple IIe, typing a letter takes about a quarter of a '
        'second to show and a new screen of text about three seconds. Typing never loses '
        'letters: if you type faster than the screen, RetroWrite II catches up when you '
        'pause. An accelerator card makes it much quicker.', BODYJ))

    chapter('D', 'Messages', [ix('messages') +
            'These are the messages RetroWrite II can show, in alphabetical order, and what '
            'to do about them.'])
    msgs = [
        ('A file with that name already exists. Replace it?', 'Click Replace to write over it, or Cancel and type another name.'),
        ('A hidden document could not be read back.', 'A RW2.WINn file was damaged or removed. The window starts empty; open the document again.'),
        ('A text file can’t keep fonts, sizes or styles. Save it as text anyway?', 'Click Save to lose the formatting in the text file, or Cancel and use Save As.'),
        ('Can’t write a work file in the RetroWrite II folder (disk full?).', 'Make room on the disk that holds RETROWRITE, then open the ODT document again.'),
        ('BASIC.SYSTEM was not found.', 'Copy BASIC.SYSTEM into the RETROWRITE folder or the top level of the disk.'),
        ('Can’t load part of RetroWrite II (RW2.OVLn is missing).', 'A program file is missing from the RETROWRITE folder. Copy the folder again.'),
        ('Can’t open the document / Can’t put the document away', 'The disk is full, write-protected or offline. ProDOS’s error is shown with it.'),
        ('Names are 1-15 letters, digits or periods, starting with a letter.', 'Type a ProDOS name, such as LETTER.2.'),
        ('No printer card was found in slot n.', 'Choose the right slot in Page Setup.'),
        ('Not found.', 'Find didn’t find the text anywhere in the document.'),
        ('RW2.LC or RW2.FONTS is missing from the RetroWrite II folder.', 'Copy the RETROWRITE folder again.'),
        ('RetroWrite II needs an Apple IIe with 128K.', 'The computer needs 128K (an extended 80-column card).'),
        ('Save changes to this document before closing it?', 'Save keeps your work; Don’t Save throws the changes away; Cancel goes back.'),
        ('SPELL.DICT was not found / is damaged', 'Copy SPELL.DICT into the RETROWRITE folder again.'),
        ('That disk is not a DOS 3.3 disk (or there is no disk in the drive).', 'Check the disk and the drive, then try again.'),
        ('The document is full.', 'The document has 30,460 characters. Split it into two documents.'),
        ('There is no GLOSSARY file in the RetroWrite II folder.', 'Choose Edit Glossary to make one.'),
        ('There is no Super Serial Card in slot n.', 'Choose the right slot in Communications.'),
        ('This document came from a text file, which can’t keep fonts and styles.', 'Click RetroWrite to save a RetroWrite document (with a new name), or Text to save plain text.'),
        ('This document is too large for RetroWrite II.', 'The file has more than 30,460 characters.'),
        ('This document is protected with a password.', 'Save it again without a password in your word processor.'),
        ('This document was too long for RetroWrite II: the end was left out.', 'The ODT document has more than 30,460 characters of text. Split it into smaller documents on the other computer.'),
        ('This is not an ODT document. / This ODT document is damaged.', 'The file ends in .ODT but is not a complete ODT (ZIP) file. Copy it again; save it again as ODT.'),
        ('This ODT document is compressed in a way RetroWrite II can’t read.', 'Save it again with LibreOffice, OpenOffice or Word.'),
        ('This is not a RetroWrite II document.', 'The file is another program’s document. Text files open directly.'),
        ('/RAM contains files that RetroWrite II will erase.', 'Press N, copy the files from /RAM to a disk, and start again; or press Y to go on.'),
    ]
    A(table([['Message', 'What to do']] + [[m, w] for m, w in msgs], [190, COLW - 190]))

    chapter('E', 'The Document Format', [
        'For programmers: how a RetroWrite II document is stored on disk. Documents are '
        'ProDOS files of type $50 with aux type $5257 (“RW”).'])
    A(table([['Bytes', 'Contents'],
             ['0-3', scr('RW2D')],
             ['4', 'version: 3'],
             ['5-6, 7-8', 'left and right margin, in screen pixels from the text’s left edge (80 per inch)'],
             ['9-24', 'eight tab stops, the same units, 0 = none'],
             ['25-64, 65-104', 'header and footer, ASCII, ending with 0'],
             ['105 -', 'the text']], [80, COLW - 80]))
    A(Spacer(1, 8))
    A(P('The text is ASCII with Return ($0D) between paragraphs and Tab ($09). Bytes $80 '
        'and above are format marks that apply to the text after them:', BODYJ))
    A(table([['Byte', 'Meaning'],
             ['$80 + f', 'character format f: bits 0-1 font (0 Sans, 1 Serif, 2 Mono), bits 2-3 '
              'size (0 9pt, 1 18pt, 2 27pt, 3 superscript; with bold set, subscript), '
              'bit 4 bold, bit 5 italic, bit 6 underline'],
             ['$83 + p', 'paragraph mark at the start of a paragraph: bits 2-3 alignment (left, '
              'center, right, justify), bits 4-5 spacing (single, 1½, double)']],
            [60, COLW - 60]))
