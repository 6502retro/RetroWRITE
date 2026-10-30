# RetroWrite II Owner's Manual

## RetroWrite II 2.2

**A word processor for the enhanced Apple IIe**

by 6502Retro

### Notice

RetroWrite II is new software written in 2026 for Apple II enthusiasts. It works in the tradition of MacWrite, MultiScribe and MouseWrite, but it contains no code from those programs and is not affiliated with their publishers. Apple, Apple IIe, Apple IIc, Apple IIGS, ImageWriter, ProDOS and Apple II DeskTop are trademarks of Apple Inc. LibreOffice is a trademark of The Document Foundation; Microsoft Word is a trademark of Microsoft Corporation.

RetroWrite II is provided as is, without warranty of any kind. Keep backup copies of your disks and your documents. Every screen picture in this manual was taken from RetroWrite II itself, running on an emulated Apple IIe.

## Preface: Welcome to RetroWrite II

RetroWrite II is a word processor for the enhanced Apple IIe, in the tradition of MacWrite, MultiScribe and MouseWrite. What you see on the screen is what you get on paper: three fonts, three sizes, bold, italic, underline, superscript and subscript, centered and justified paragraphs, tab stops and page breaks all show as you type, and print the same way on an ImageWriter II or an Epson printer.

You work with the mouse and pull-down menus, or with the keyboard: every menu can be opened with the ESC key, and the most used commands have an Open-Apple-key shortcut. RetroWrite II is made to live on a hard disk; the parts of the program that you use less often are read from the disk when you need them, which leaves room in memory for long documents.

Documents are saved as ODT, the OpenDocument format of LibreOffice and OpenOffice, which Microsoft Word opens too, so the letters and newsletters you write on the Apple IIe can go straight to a modern computer, and documents from a modern computer can come back.

### What you need

- An enhanced Apple IIe (65C02) with 128K of memory (an extended 80-column card). It should also run on an Apple IIc or IIgs, but it was made and tested on the IIe.
- An AppleMouse or compatible mouse card: you need the mouse to select text, although every command can also be given from the keyboard.
- A hard disk, or a large disk image, with ProDOS 8. RetroWrite II takes about 600K including its dictionary.
- A monochrome monitor gives the sharpest text. A color monitor also works.
- For printing: an ImageWriter II or an Epson-compatible printer on a parallel or serial printer card.

### How this manual is organized

- **Chapter 1, Getting Started**, tells you how to install and start RetroWrite II and what you see on the screen.
- **Chapter 2, A Quick Tour**, has you write, style, save and print a short letter.
- **Chapters 3 to 11** are the reference section, one chapter per menu from the Apple menu to the Windows menu, with an entry for every command.
- **The appendixes** list every key, the files RetroWrite II uses, its limits, its messages and the document format.

Each command entry starts with the menu it is in and its key, if it has one. Then come what the command does, the steps to give it, and notes, and it ends with the related entries.

### Conventions

- **Open-Apple** (the key with the outlined apple, at the left of the space bar) is held down while you press another key. "Open-Apple-S" means hold Open-Apple and press S. **Solid-Apple**, at the right of the space bar, works the same way.
- **RETURN**, **ESC**, **TAB**, **DELETE** and the arrow keys are shown in capitals.
- Text you type, and names you see on the screen, are shown `like this`.
- Choosing a command means pulling down its menu and selecting it, with the mouse or with the keyboard.

> **By the Way:** Notes like this one give helpful extra information.

> **Important!** Notes like this one tell you something you need to know to keep your work safe.

## Chapter 1: Getting Started

This chapter explains how to install RetroWrite II on your hard disk, how to start it, and what you see on the screen.

### Installing RetroWrite II

RetroWrite II comes installed on the disk image it was delivered on, in the folder `/DESKTOP/RETROWRITE`. To put it on another hard disk, copy the whole `RETROWRITE` folder there, for example with Apple II DeskTop. Everything RetroWrite II needs is in that folder; it finds the folder by itself wherever you put it.

On a PC you can also use the included installer, which copies the files into a disk image and adds RetroWrite II to Apple II DeskTop’s Shortcuts menu:

`python3 install_rw2.py MYDISK.po --ac ac.jar --files .`

### Starting RetroWrite II

In Apple II DeskTop, open the `RETROWRITE` folder and double-click `RW2.SYSTEM`, or choose RetroWrite II from the Shortcuts menu. From BASIC.SYSTEM type `-/DESKTOP/RETROWRITE/RW2.SYSTEM`; from a program selector such as Bitsy Bye, choose `RW2.SYSTEM`.

A welcome picture may appear for a few seconds while the program loads; press any key to go on. Then you see an empty, untitled document ready for typing.

> **By the Way:** RetroWrite II uses all of the Apple IIe’s auxiliary memory, so it switches off the /RAM disk while it runs and switches it back on when you quit. If /RAM has files in it, RetroWrite II warns you first and lets you stop.

### The RetroWrite II screen

Here is RetroWrite II with a letter on the screen and the ruler showing:

![Figure 1-1: The RetroWrite II screen|100](letter.png)

**The menu bar** holds the menus: the Apple menu (the apple at the left end), File, Edit, Search, Format, Font, Size, Style and Windows. **The ruler** (shown with Open-Apple-H) sets margins, tab stops, alignment and line spacing. **The text area** shows your document as it will print. **The scroll bar** at the right moves through a long document. **The pointer** follows the mouse: an arrow over the menus and the scroll bar, an I-beam over the text. **The caret**, a blinking line, shows where the next letter you type goes.

### Using the mouse

RetroWrite II works like the programs you may know from the Macintosh and the Apple IIgs:

- **Point** by moving the mouse. **Click**: press and release the button. **Double-click**: click twice quickly. **Drag**: hold the button down while you move the mouse.
- **Choosing from a menu:** point at a menu title, hold the button down, drag down to the command and let go. Or click the title once: the menu stays open until you click a command (or click outside to close it). Commands you can’t use are dimmed; a check mark shows a setting that is on.
- **Placing the caret:** click in the text.
- **Selecting text:** drag across it. Double-click a word to select the word; keep the button down after the second click and drag to select whole words. Hold down Open-Apple or Solid-Apple and click to extend a selection to the place you click.
- **Scrolling:** click a scroll arrow to move one line (hold it down to keep scrolling), click the gray area to move a page, or drag the scroll box.

### Using the keyboard

Type as on a typewriter. You never press RETURN at the end of a line: RetroWrite II moves words to the next line by itself (word wrap). Press RETURN only to end a paragraph. The arrow keys move the caret; DELETE erases the character before the caret, or the selected text.

**ESC opens the menus from the keyboard.** The File menu opens first; use the left and right arrows to move to another menu, the up and down arrows to choose a command, and RETURN to give it. Typing the first letter of a command also gives it. Press ESC again to close the menu. Appendix A lists every key.

![Figure 1-2: The File menu, opened with ESC, with Open... chosen with the arrow keys|62](m_file_sel-f3ba.png)

### Leaving RetroWrite II

Choose Quit from the File menu (Open-Apple-Q). If a document has changes you haven’t saved, RetroWrite II asks whether to save them. You return to the program you started RetroWrite II from, such as Apple II DeskTop.

## Chapter 2: A Quick Tour

In this chapter you write the short club letter shown below, give it fonts and styles, set its tabs, save it and print it. It takes about fifteen minutes. The finished letter is also in the `DOCUMENTS` folder, as `LETTER`, if you want to look at it first.

![Figure 2-1: The finished letter|100](letter.png)

### Step 1: Type the text

Start RetroWrite II; an empty document appears. Type the first line, `6502 Retro Computing`, and press RETURN. Type the second line and press RETURN twice. Then type the rest of the letter. Don’t press RETURN inside a paragraph: let the words wrap by themselves. Press RETURN at the end of each paragraph and once more for an empty line.

If you make a mistake, press DELETE to erase back to it, or click just after the mistake, press DELETE and type the correction.

### Step 2: Center the heading

Click anywhere in the first line and choose **Center** from the Format menu (Open-Apple-E). Do the same for the second line. Alignment belongs to the whole paragraph, so you don’t need to select the text.

### Step 3: Change fonts, sizes and styles

Drag across the first line to select it. Choose **Serif** from the Font menu, **18 Point** from the Size menu and **Bold** from the Style menu (Open-Apple-B). Select the second line and choose Serif and **Italic** (Open-Apple-I).

Double-click the word `Saturday` and press Open-Apple-B to make it bold. Select the word `Agenda` and choose **Underline** (Open-Apple-U).

You can also choose the style before you type: with nothing selected, choose Bold, type the words, and choose **Plain Text** (Open-Apple-T) to go back to normal letters.

![Figure 2-2: The Style menu. The check mark shows the style of the selected text.|62](m_style-e282.png)

### Step 4: Set tab stops for the date and the agenda

Choose **Show Ruler** from the Format menu (Open-Apple-H). Click the ruler’s scale at ½ inch and at 1 inch to put two tab stops there, and at about 3¾ inches for the date. Now click at the start of the date, press TAB three times, and the date moves to the third stop. In the agenda lines, press TAB before the number and again after it.

Click in the long paragraph and click the fourth alignment icon in the ruler (or choose **Justify**, Open-Apple-J) so that both of its edges are straight.

### Step 5: Save the letter

Choose **Save** from the File menu (Open-Apple-S). Because the letter has no name yet, the Save dialog appears. Type a name, such as `MYLETTER`, and click **Save**. RetroWrite II adds `.ODT` and saves the letter as an ODT document, which you can also open with LibreOffice or Word on a modern computer. The name appears at the top of the File menu and in the Windows menu.

### Step 6: Print it

Turn on your printer and choose **Print** from the File menu (Open-Apple-P). The first time, click **Setup...** and choose your printer and the slot of its card, then click OK and choose Print again. RetroWrite II shows the page it is printing; press ESC to stop.

That’s all there is to it. To see more, choose **Mouse Tutorial** from the Apple menu: it opens a document that shows each feature, which you can change as much as you like without harming the original.

## Chapter 3: The Apple Menu

The Apple menu, at the left end of the menu bar, holds information about RetroWrite II, the Mouse Tutorial, your Startup Preferences and the desk accessories: small programs you can use without leaving your work.

![Figure 3-1: The Apple menu|62](m_apple-f3ba.png)

### About RetroWrite II

<p class="meta">Apple menu</p>

Shows the version of RetroWrite II and who made it.

1. Choose About RetroWrite II... from the Apple menu.
2. Click OK (or press RETURN).

<p class="see">See also: Keyboard Help</p>

![Figure 3-2: About RetroWrite II|58](about-ea1e.png)

### Keyboard Help

<p class="meta">Apple menu &middot; Open-Apple-?</p>

Shows a short list of the keyboard commands.

1. Choose Keyboard Help from the Apple menu, or press Open-Apple-?.
2. Click OK when you have read it.

> **By the Way:** You don’t need to hold Shift: Open-Apple-/ works too. Appendix A has the full list.

<p class="see">See also: Appendix A, Keyboard Summary</p>

### Mouse Tutorial

<p class="meta">Apple menu</p>

Opens the TUTORIAL document: a guided tour of RetroWrite II that shows each feature in a real document.

1. Choose Mouse Tutorial from the Apple menu.
2. Read it, scroll with the scroll bar, and try things out in it.

> **By the Way:** The tutorial opens as an untitled copy, so you can change it freely; the original file is never changed. If you save it, give it a new name.

<p class="see">See also: Chapter 2, A Quick Tour</p>

### Startup Preferences

<p class="meta">Apple menu</p>

Sets how RetroWrite II starts: whether the ruler shows, the font and size of new documents, the folder the Open and Save dialogs start in, and the printer settings from Page Setup. They are kept in the file `RW2.PREFS`.

1. Choose Startup Preferences... from the Apple menu.
2. Click the choices you want: **Show the ruler** Yes or No; **New documents** Sans, Serif or Mono; **Size** 9, 18 or 27 Point.
3. Click **Save** to keep them, or Cancel.

> **By the Way:** **Documents** shows the folder the Open and Save dialogs will start in: the last folder you used. Clicking Save also keeps the printer, slot and print quality you chose in Page Setup, so set those first.

<p class="see">See also: Page Setup; Show Ruler</p>

![Figure 3-3: Startup Preferences|60](prefs-a662.png)

### Load DOS 3.3 File

<p class="meta">Apple menu</p>

Reads a text file from a DOS 3.3 disk in a 5.25-inch drive and puts it into your document at the caret. Use it for letters and notes you wrote years ago with DOS 3.3 programs or Applesoft.

1. Put the DOS 3.3 disk in drive 1 or 2 of the disk card in slot 6.
2. Click where the text should go in your document.
3. Choose Load DOS 3.3 File... from the Apple menu.
4. Click **S6 D1** or **S6 D2** for the drive.
5. The text files on the disk are listed. Click one and click it again, or use the arrow keys and press RETURN. ESC cancels.

> **By the Way:** Only text files (DOS type T) are listed, up to 24. Lines end where the original had RETURN; the text takes the style of the text at the caret.

<p class="see">See also: Open</p>

![Figure 3-4: Choosing a text file from a DOS 3.3 disk|58](dos2-6e8b.png)

### Communications

<p class="meta">Apple menu</p>

Turns RetroWrite II into a simple terminal for a Super Serial Card, so you can talk to another computer or a modem. The text you receive can be kept in your document.

1. Choose Communications... from the Apple menu.
2. Click the **Card slot** of your Super Serial Card and the **Speed** (300, 1200, 2400, 9600 or 19200 baud).
3. Choose **Keep the text** Yes if what you receive should go into your document at the caret.
4. Click **Connect**. What you type is sent; what comes in is shown.
5. Press Open-Apple-. (period) to end.

> **By the Way:** The card is set to 8 data bits, no parity, 1 stop bit. If no Super Serial Card is in the slot you chose, RetroWrite II tells you so.

<p class="see">See also: Startup Preferences</p>

![Figure 3-5: Communications settings|60](comm-a0bd.png)

### BASIC.SYSTEM

<p class="meta">Apple menu</p>

Leaves RetroWrite II for Applesoft BASIC (BASIC.SYSTEM). When you type `BYE` in BASIC you come straight back to RetroWrite II.

1. Choose BASIC.SYSTEM from the Apple menu.
2. If a document has unsaved changes, you are asked whether to save it.
3. Click **Go**. The BASIC prompt `]` appears.
4. When you are done, type `BYE` and press RETURN.

> **By the Way:** RetroWrite II starts again with an empty document; open your document again with Open. BASIC.SYSTEM must be in the RETROWRITE folder or at the top level of the disk.

<p class="see">See also: Quit</p>

## Chapter 4: The File Menu

The File menu creates, opens, saves, prints and closes documents. Its first line shows the name of the document on the screen.

![Figure 4-1: The File menu|62](m_file-f3ba.png)

### New

<p class="meta">File menu &middot; Open-Apple-N</p>

Starts a new, empty document called Untitled.

1. Choose New from the File menu, or press Open-Apple-N.

> **By the Way:** If the document on the screen has text in it, the new document opens in a new window and the old one stays open in the Windows menu (up to four documents). If all four windows are in use, you are asked whether to save the changes to the one on the screen, which the new document then replaces.

<p class="see">See also: The Windows Menu; Close</p>

### Open

<p class="meta">File menu &middot; Open-Apple-O</p>

Opens a RetroWrite II document, an ODT document (LibreOffice, OpenOffice or Word) or a text file from a disk.

1. Choose Open... from the File menu, or press Open-Apple-O.
2. The dialog lists the folders (□ folder icons) and documents (page icons) in the current folder. Click a document and click **Open**, or double-click it.
3. To open a folder, double-click it. Click **Up** to go to the folder that holds this one, and **Disks** to see all the disks.

**Example.** To open the sample letter, press Open-Apple-O, type `L` (the list jumps to the first name starting with L) and press RETURN.

> **By the Way:** Keyboard: up and down arrows choose, right arrow or RETURN opens, left arrow goes up a folder, ESC cancels. Only folders, RetroWrite II documents, ODT documents (names ending in .ODT) and text files are listed, up to 48 per folder. A text file opens as plain text in Sans 9 point. An ODT document shows a bar while it is unpacked; see “ODT documents” below.

<p class="see">See also: New; Save; Startup Preferences</p>

![Figure 4-2: The Open dialog|60](open-0d08.png)

### Close

<p class="meta">File menu</p>

Closes the document on the screen.

1. Choose Close from the File menu.
2. If there are changes, click **Save**, **Don’t Save** or **Cancel**.

> **By the Way:** If other documents are open, the next one comes on the screen. If it was the only one, you get an empty Untitled document.

<p class="see">See also: The Windows Menu; Quit</p>

![Figure 4-3: Asked when a changed document would be closed|56](confirm-77f7.png)

### Save

<p class="meta">File menu &middot; Open-Apple-S</p>

Saves the document on the screen under its name.

1. Choose Save from the File menu, or press Open-Apple-S.

> **By the Way:** If the document has no name yet, the Save As dialog appears. A document that came from an ODT document is saved as ODT again. A document that came from a text file is saved as a text file again; if you have added fonts or styles to it, RetroWrite II asks whether to save it as a RetroWrite document (which keeps them) or as text. Save often!

<p class="see">See also: Save As; Save As Text</p>

### Save As

<p class="meta">File menu &middot; Open-Apple-A</p>

Saves the document under a new name or in another folder, normally as an ODT document that LibreOffice, OpenOffice and Word can open. It keeps the fonts, styles, ruler, header and footer.

1. Choose Save As... from the File menu, or press Open-Apple-A.
2. Open the folder you want, as in the Open dialog.
3. Type the name in the **Name** box (DELETE erases). Clicking a document in the list copies its name. A name without a period gets `.ODT` added.
4. Click **Save** or press RETURN.

> **By the Way:** Names have up to 15 letters, digits and periods and start with a letter, as ProDOS requires. If a file with that name exists, you are asked whether to replace it. To save in RetroWrite II’s own format instead, give a name with any other ending, such as `LETTER.RW`; these are ProDOS type $50 with aux type $5257.

<p class="see">See also: Save; Save As Text</p>

![Figure 4-4: The Save As dialog|60](saveas-0a6c.png)

### Save As Text

<p class="meta">File menu</p>

Saves the document as a plain ProDOS text file (TXT), which other programs such as AppleWorks, BASIC and Apple II DeskTop can read.

1. Choose Save As Text... from the File menu.
2. Choose the folder and type a name, then click Save.

> **By the Way:** A text file can’t keep fonts, sizes, styles, the ruler or page breaks; RetroWrite II warns you if the document has any. Each paragraph becomes one line ending with RETURN.

<p class="see">See also: Save As</p>

### ODT documents

ODT (OpenDocument Text) is the standard document format of LibreOffice and OpenOffice; Microsoft Word, Google Docs and Apple Pages open it too. RetroWrite II saves and opens ODT documents itself, so you can carry your work between the Apple IIe and a modern computer: copy the file with a disk-image tool such as CiderPress II or AppleCommander, or a shared hard disk, and double-click it there. Going the other way, save from your word processor as ODT, with a name of up to 15 letters, digits and periods ending in `.ODT`.

| RetroWrite II | In the ODT document |
| --- | --- |
| Sans, Serif, Mono | Liberation Sans, Liberation Serif, Liberation Mono (other fonts open as the nearest of the three) |
| 9, 18 and 27 point | 12, 24 and 36 point, which keeps lines about as long as RetroWrite II prints them (opening: up to 17 point is 9, up to 29 is 18) |
| Bold, italic, underline, superscript, subscript | the same |
| Alignment, line spacing, page breaks | the same |
| Ruler margins and tab stops | paragraph indents and tab stops |
| Header and footer, # for the page number | page header and footer with a page-number field |
| Letters | plain ASCII; when opening, accented letters lose their accents and curly quotes and dashes become straight ones |

> **By the Way:** When RetroWrite II opens an ODT document it leaves out pictures, footnotes and comments (the text in tables is kept, one paragraph per cell), and list items start with a dash. Opening takes time on a 1 MHz Apple IIe: about half a minute for a short letter saved by LibreOffice, and less for documents RetroWrite II saved itself, which are not compressed. Two work files, `RW2.XML1` and `RW2.XML2`, are made in the RETROWRITE folder while a document is opened or saved, so the disk needs some free space.

![Figure 4-5: Opening an ODT document|52](odtopen-68b1.png)

![Figure 4-6: A document from LibreOffice, opened in RetroWrite II|100](odtnews.png)

### Page Setup

<p class="meta">File menu</p>

Chooses your printer and how to print, and sets the header, footer and page numbers of the document.

1. Choose Page Setup... from the File menu.
2. Click your **Printer**: ImageWriter II, Epson, or Plain text (any printer, no styles).
3. Click the **Slot** of the printer card.
4. Choose **Print**: **As on screen** prints in graphics, exactly as you see it; **Faster (printer’s font)** uses the printer’s own letters, with bold, underline, superscript and subscript.
5. **Page numbers** Yes prints `- 1 -` at the foot of each page when there is no footer.
6. Click the **Header** or **Footer** box to type a line printed at the top or bottom of every page. A `#` in it prints the page number.
7. Click **OK**.

**Example.** A footer of `Club newsletter - page #` prints `Club newsletter - page 1`, `page 2` ...

> **By the Way:** The header and footer are saved with the document. The printer, slot and print choices last until you quit; to keep them, click Save in Startup Preferences.

<p class="see">See also: Print; Startup Preferences; Insert Page Break</p>

![Figure 4-7: Page Setup|62](pagesetup-7c7d.png)

### Print

<p class="meta">File menu &middot; Open-Apple-P</p>

Prints the document on the screen.

1. Turn on the printer and make sure it is selected (on line).
2. Choose Print... from the File menu, or press Open-Apple-P.
3. RetroWrite II names the printer and slot; click **Print**. Click **Setup...** to change them first.
4. A box shows the page being printed. Press ESC to stop.

> **By the Way:** Pages are 11 inches long with the text 6.4 inches wide, like the ruler. Printing **As on screen** sends the page as graphics, which takes longer than **Faster**. If no printer card is in the slot, RetroWrite II says so.

<p class="see">See also: Page Setup; Print Several</p>

![Figure 4-8: Before printing|56](print-77f7.png)

### Print Several

<p class="meta">File menu</p>

Prints a series of documents, one after the other, without opening each one yourself.

1. Choose Print Several... from the File menu.
2. In the dialog, open the first document to print; it is printed.
3. The dialog appears again: choose the next one, and so on.
4. Click **Cancel** when you are done. Your own document comes back on the screen.

> **By the Way:** All the documents are printed with the printer settings in Page Setup and each document’s own header and footer.

<p class="see">See also: Print</p>

### Quit

<p class="meta">File menu &middot; Open-Apple-Q</p>

Leaves RetroWrite II.

1. Choose Quit from the File menu, or press Open-Apple-Q.
2. For each open document with unsaved changes, click Save or Don’t Save (Cancel stops quitting).

> **By the Way:** You return to the program that started RetroWrite II, such as Apple II DeskTop.

<p class="see">See also: Close</p>

## Chapter 5: The Edit Menu

The Edit menu moves and copies text through the clipboard, selects the whole document and types glossary entries for you.

![Figure 5-1: The Edit menu|62](m_edit-f3ba.png)

### Cut

<p class="meta">Edit menu &middot; Open-Apple-X</p>

Removes the selected text and puts it on the clipboard, ready to paste.

1. Select the text.
2. Choose Cut from the Edit menu, or press Open-Apple-X.

> **By the Way:** The clipboard holds one piece of text at a time, with its fonts and styles. It is kept in the file `RW2.CLIPBOARD`, so it is still there after you quit and start RetroWrite II again, or open another document.

<p class="see">See also: Copy; Paste; Clear</p>

### Copy

<p class="meta">Edit menu &middot; Open-Apple-C</p>

Puts a copy of the selected text on the clipboard, leaving the document as it is.

1. Select the text.
2. Choose Copy from the Edit menu, or press Open-Apple-C.

<p class="see">See also: Cut; Paste</p>

### Paste

<p class="meta">Edit menu &middot; Open-Apple-V</p>

Puts the text on the clipboard into the document at the caret, or in place of the selected text.

1. Click where the text should go, or select text to replace.
2. Choose Paste from the Edit menu, or press Open-Apple-V.

**Example.** To move a paragraph: select it, press Open-Apple-X, click where it should go and press Open-Apple-V.

> **By the Way:** You can paste the same text as many times as you like.

<p class="see">See also: Cut; Copy</p>

### Clear

<p class="meta">Edit menu</p>

Removes the selected text without putting it on the clipboard.

1. Select the text.
2. Choose Clear from the Edit menu, or press DELETE.

<p class="see">See also: Cut</p>

### Select All

<p class="meta">Edit menu &middot; Open-Apple-W</p>

Selects the whole document.

1. Choose Select All from the Edit menu, or press Open-Apple-W.

**Example.** To change the font of the whole document: press Open-Apple-W, then choose the font.

<p class="see">See also: Using the mouse (chapter 1)</p>

### Glossary

<p class="meta">Edit menu &middot; Control-G</p>

Types a stored word or phrase for you. You give each phrase a one-key name in the glossary; Control-G and that key put it into your document at the caret.

1. Hold down Control and press G (or choose Glossary from the Edit menu).
2. A box asks for the entry’s key. Press it. ESC cancels.

**Example.** With the entry `A=Apple IIe` in the glossary, Control-G then `A` types `Apple IIe`.

> **By the Way:** Press `?` instead of a key to see the list of keys. Letters can be typed in capitals or small letters. The text takes the current typing style. The glossary that comes with RetroWrite II has A, R, D, Y and S.

<p class="see">See also: Edit Glossary</p>

![Figure 5-2: Waiting for the glossary key|56](gloss-ee52.png)

### Edit Glossary

<p class="meta">Edit menu</p>

Opens the glossary file so you can add, change or remove entries.

1. Choose Edit Glossary from the Edit menu. The `GLOSSARY` file opens as a text document.
2. Each line is one entry: the key, an equal sign and the text, such as `Y=Yours sincerely,`.
3. Change the lines, then choose Save (Open-Apple-S).

> **By the Way:** Each entry fits on one line, up to about 190 characters. A key can be any letter, digit or sign. The file is in the RETROWRITE folder; if it doesn’t exist, Edit Glossary starts a new one.

<p class="see">See also: Glossary</p>

## Chapter 6: The Search Menu

The Search menu finds and replaces words and checks your spelling.

![Figure 6-1: The Search menu|62](m_search-f3ba.png)

### Find

<p class="meta">Search menu &middot; Open-Apple-F</p>

Finds a word or phrase in the document and selects it.

1. Choose Find... from the Search menu, or press Open-Apple-F.
2. Type what to find (up to 30 characters) and click OK or press RETURN. The box shows what you looked for last time; press DELETE to erase it.

**Example.** Find `the` finds `The`, `the` and `weather`.

> **By the Way:** Find ignores capitals and fonts and also finds the letters inside longer words. It searches from the selection to the end, then from the top. If the text is not in the document, RetroWrite II says Not found.

<p class="see">See also: Find Next; Replace</p>

![Figure 6-2: Find|56](find-fb17.png)

### Find Next

<p class="meta">Search menu &middot; Open-Apple-G</p>

Finds the next place with the same text you last looked for.

1. Choose Find Next from the Search menu, or press Open-Apple-G.

<p class="see">See also: Find</p>

### Replace

<p class="meta">Search menu &middot; Open-Apple-R</p>

Finds a word or phrase and replaces it with another, one by one or all at once.

1. Choose Replace... from the Search menu, or press Open-Apple-R.
2. Type what to find and press RETURN, then what to put in its place and press RETURN.
3. Each time the text is found, it is selected and you are asked: **Replace** replaces it and goes on, **All** replaces it everywhere in the document, **Stop** stops.

> **By the Way:** At the end RetroWrite II tells you how many times it replaced the text. The replacement takes the style of the text it replaces.

<p class="see">See also: Find</p>

![Figure 6-3: Replacing one at a time|56](replace-77f7.png)

### Check Spelling

<p class="meta">Search menu &middot; Open-Apple-K</p>

Checks every word against the 460K dictionary SPELL.DICT and your own user dictionary, and shows you the words it doesn’t know.

1. Click where checking should start (at the end of the document, it checks everything).
2. Choose Check Spelling from the Search menu, or press Open-Apple-K.
3. For each unknown word, which is selected in your text, click:
4. **Skip** to leave it; **Replace** to type the correct word; **Ignore All** to skip it everywhere until you quit; **Learn** to add it to your user dictionary; **Suggest** (or press G) to see likely words; **Stop** (or ESC) to stop.
5. To use a suggestion, click it or press its number, 1 to 9.

> **By the Way:** A replacement keeps the capitals of the original word. When the end is reached, you can go on from the top. Words with digits are skipped.

<p class="see">See also: Edit User Dictionary</p>

![Figure 6-4: An unknown word, with suggestions|62](spellsugg-1fee.png)

### Edit User Dictionary

<p class="meta">Search menu</p>

Opens your user dictionary, USER.DICT, the list of words you taught the spelling checker with Learn.

1. Choose Edit User Dictionary from the Search menu.
2. The words appear one to a line. Change, add or remove words.
3. Choose Save (Open-Apple-S).

> **By the Way:** Words are kept in small letters. Names and special words you use often are good to add here.

<p class="see">See also: Check Spelling</p>

## Chapter 7: The Format Menu

The Format menu and the ruler shape your paragraphs: margins, tab stops, alignment, line spacing and page breaks. Margins and tab stops belong to the whole document. Alignment and spacing belong to each paragraph: they apply to the paragraph with the caret, or to every paragraph in the selection.

![Figure 7-1: The Format menu|62](m_format-edf3.png)

### Show Ruler

<p class="meta">Format menu &middot; Open-Apple-H</p>

Shows or hides the ruler at the top of the window.

1. Choose Show Ruler from the Format menu, or press Open-Apple-H. A check mark shows when the ruler is on; choose it again to hide the ruler.

<p class="see">See also: The ruler (below); Startup Preferences</p>

![Figure 7-2: The ruler|68](letter-18f6.png)

### Using the ruler

The scale is marked in inches, from 0 to 6.4. The two triangles under it are the left and right margins; the small marks are tab stops.

- **Margins:** drag a triangle left or right. The margins are at least ½ inch apart.
- **A new tab stop:** click the scale where you want it. You can have up to 8.
- **Moving a tab stop:** drag it along the scale.
- **Removing a tab stop:** drag it down off the ruler.
- **Alignment:** click one of the four icons: left, centered, right or justified.
- **Line spacing:** click one of the three icons: single, 1½ or double.
> **By the Way:** Positions snap to 1/16 inch. Pressing TAB moves to the next tab stop; past the last one, or if there are none, it moves to the next ½ inch. The ruler is saved with the document.

### Align Left

<p class="meta">Format menu &middot; Open-Apple-L</p>

Lines up the paragraph’s lines at the left margin, with a ragged right edge. This is how new paragraphs start.

1. Click in the paragraph (or select several).
2. Choose Align Left from the Format menu, press Open-Apple-L, or click the first alignment icon in the ruler.

<p class="see">See also: Center; Align Right; Justify</p>

### Center

<p class="meta">Format menu &middot; Open-Apple-E</p>

Centers each line of the paragraph between the margins. Use it for titles and headings.

1. Click in the paragraph.
2. Choose Center from the Format menu or press Open-Apple-E.

<p class="see">See also: Align Left</p>

### Align Right

<p class="meta">Format menu</p>

Lines up the paragraph’s lines at the right margin, for dates and addresses at the right side of a letter.

1. Click in the paragraph.
2. Choose Align Right from the Format menu.

<p class="see">See also: Align Left</p>

### Justify

<p class="meta">Format menu &middot; Open-Apple-J</p>

Spreads the words of each line so both edges of the paragraph are straight, as in a book. The last line of the paragraph stays at the left.

1. Click in the paragraph.
2. Choose Justify from the Format menu or press Open-Apple-J.

<p class="see">See also: Align Left</p>

### Single, 1½ and Double Space

<p class="meta">Format menu</p>

Sets the space between the lines of the paragraph.

1. Click in the paragraph or select several.
2. Choose Single Space, 1 1/2 Space or Double Space from the Format menu, or click a spacing icon in the ruler.

> **By the Way:** Double spacing is handy for drafts that others will mark up on paper.

<p class="see">See also: Using the ruler</p>

### Insert Page Break

<p class="meta">Format menu</p>

Makes the text after the caret start on a new page when you print.

1. Click where the new page should start.
2. Choose Insert Page Break from the Format menu.

> **By the Way:** A page break shows as a dotted line across the window. To remove it, click just after it and press DELETE. Without page breaks, RetroWrite II starts a new page when one is full.

<p class="see">See also: Print</p>

## Chapter 8: The Font Menu

RetroWrite II has three fonts, all designed for the Apple IIe’s double hi-res screen and printed as you see them.

![Figure 8-1: The Font menu|62](m_font-67ba.png)

| Font | Looks like | Good for |
| --- | --- | --- |
| Sans | clean letters without serifs | most writing, headings |
| Serif | letters with small serifs, like a book | letters, reports, titles |
| Mono | every letter the same width, like a typewriter | tables, lists that must line up, program listings |

### Sans, Serif, Mono

<p class="meta">Font menu</p>

Changes the font of the selected text, or of the text you type next.

1. Select the text (or, to choose the font for what you type next, just put the caret where you will type).
2. Choose Sans, Serif or Mono from the Font menu.

> **By the Way:** The check mark shows the font at the start of the selection. The font for new documents is set in Startup Preferences.

<p class="see">See also: The Size Menu; The Style Menu; Startup Preferences</p>

## Chapter 9: The Size Menu

Each font comes in three sizes. 18 and 27 point are the 9 point letters drawn two and three times as large, for headings and titles.

![Figure 9-1: The Size menu|62](m_size-da47.png)

### 9, 18 and 27 Point

<p class="meta">Size menu</p>

Changes the size of the selected text, or of the text you type next.

1. Select the text.
2. Choose 9 Point, 18 Point or 27 Point from the Size menu.

> **By the Way:** A line is as tall as its largest letters. With Faster printing, 18 and 27 point print as wide printer letters.

<p class="see">See also: The Font Menu</p>

## Chapter 10: The Style Menu

Styles change the look of letters in any font and size. You can combine bold, italic and underline.

![Figure 10-1: The Style menu|62](m_style-e282.png)

### Plain Text

<p class="meta">Style menu &middot; Open-Apple-T</p>

Removes bold, italic, underline, superscript and subscript from the selection.

1. Select the text.
2. Choose Plain Text from the Style menu or press Open-Apple-T.

<p class="see">See also: Bold, Italic, Underline</p>

### Bold, Italic, Underline

<p class="meta">Style menu &middot; Open-Apple-B   Open-Apple-I   Open-Apple-U</p>

Makes letters heavier, slanted, or underlined. Each command turns its style on for the selection, or off if the selection already has it.

1. Select the text.
2. Choose Bold, Italic or Underline from the Style menu, or press Open-Apple-B, Open-Apple-I or Open-Apple-U.

**Example.** Press Open-Apple-B, type `important`, press Open-Apple-B again, and type on: only the word is bold.

> **By the Way:** With nothing selected, the style applies to what you type next. Check marks show the styles of the selected text.

<p class="see">See also: Plain Text</p>

### Superscript, Subscript

<p class="meta">Style menu</p>

Makes small letters raised above the line (superscript), as in E=mc² or 2<super>nd</super>, or lowered below it (subscript), as in H<sub>2</sub>O.

1. Select the characters.
2. Choose Superscript or Subscript from the Style menu.
3. To go back to normal letters, choose Plain Text.

<p class="see">See also: Plain Text</p>

## Chapter 11: The Windows Menu

RetroWrite II can have up to four documents open at once. One is on the screen; the others wait in the Windows menu. You can copy and paste between them.

![Figure 11-1: The Windows menu|62](m_windows2-e282.png)

### Switching windows

<p class="meta">Windows menu</p>

Brings another open document onto the screen.

1. Open the Windows menu: it lists the open documents, with a check mark beside the one on the screen.
2. Choose the document you want.

> **By the Way:** New and Open use a new window when the document on the screen has something in it. While a document is not on the screen, it waits on the hard disk in a file named `RW2.WIN0` to `RW2.WIN3` in the RETROWRITE folder, with all its changes, so no extra memory card is needed.

<p class="see">See also: New; Open; Close</p>

## Appendix A: Keyboard Summary

Hold down Open-Apple or Solid-Apple and press the key shown.

| Keys | Command | Keys | Command |
| --- | --- | --- | --- |
| Open-Apple-N | New | Open-Apple-H | Show Ruler |
| Open-Apple-O | Open | Open-Apple-L | Align Left |
| Open-Apple-S | Save | Open-Apple-E | Center |
| Open-Apple-A | Save As | Open-Apple-J | Justify |
| Open-Apple-P | Print | Open-Apple-T | Plain Text |
| Open-Apple-Q | Quit | Open-Apple-B | Bold |
| Open-Apple-X | Cut | Open-Apple-I | Italic |
| Open-Apple-C | Copy | Open-Apple-U | Underline |
| Open-Apple-V | Paste | Open-Apple-? | Keyboard Help |
| Open-Apple-W | Select All | Control-G | Glossary |
| Open-Apple-F | Find | ESC | open the menus |
| Open-Apple-G | Find Next |  |  |
| Open-Apple-R | Replace |  |  |
| Open-Apple-K | Check Spelling |  |  |

### Moving and editing

| Keys | What they do |
| --- | --- |
| ← → | one character left or right |
| ↑ ↓ | one line up or down |
| Open-Apple-←  Open-Apple-→ | one word left or right |
| Open-Apple-↑  Open-Apple-↓ | one screen up or down |
| Open-Apple-1  Open-Apple-9 | to the top or the end of the document |
| RETURN | end the paragraph |
| TAB | to the next tab stop |
| DELETE | erase the character before the caret, or the selection |
| Open-Apple-Delete | erase the character after the caret |

### Menus and dialogs

| Keys | What they do |
| --- | --- |
| ESC | open the menus (File first); in a menu: close it |
| ← → in a menu | the menu to the left or right |
| ↑ ↓ in a menu | choose a command |
| RETURN | give the chosen command; in a dialog: the button with the heavy border |
| a letter in a menu | the command that starts with that letter |
| a letter in a dialog | the button that starts with that letter (not in a dialog with a box you type into) |
| ESC in a dialog | Cancel (or Stop) |

### Mouse

| Action | What it does |
| --- | --- |
| click | place the caret |
| drag | select text |
| double-click | select a word |
| double-click and drag | select whole words |
| Open-Apple or Solid-Apple click | extend the selection |
| click a menu title | open the menu (it stays open) |

## Appendix B: Files and Folders

Everything RetroWrite II uses is in one folder, normally `/DESKTOP/RETROWRITE`. Keep these files together; RetroWrite II finds the folder by itself.

| File | What it is |
| --- | --- |
| `RW2.SYSTEM` | the program you start; shows the welcome picture |
| `RW2.MAIN` | the main part of the program |
| `RW2.LC`, `RW2.FONTS` | more of the program, and the fonts |
| `RW2.OVL1` - `RW2.OVL9`, `RW2.OVLA`, `RW2.OVLB` | parts read from the disk when needed: files, spelling, editing, menus, printing, desk accessories, dialogs, and saving and opening ODT documents |
| `RW2.SPLASH` | the welcome picture |
| `SPELL.DICT` | the dictionary (460K) |
| `TUTORIAL` | the Mouse Tutorial document |
| `GLOSSARY` | your glossary (a text file) |
| `BASIC.SYSTEM` | for Apple menu → BASIC.SYSTEM (optional) |

These files are made as you work:

| File | What it is |
| --- | --- |
| `USER.DICT` | your own words for the spelling checker (Learn) |
| `RW2.PREFS` | your Startup Preferences |
| `RW2.CLIPBOARD` | the clipboard |
| `RW2.WIN0` - `RW2.WIN3` | documents waiting in other windows |
| `RW2.QUIT` | only while you are in BASIC.SYSTEM |
| `RW2.XML1`, `RW2.XML2` | work files, only while an ODT document is opened or saved |

Your documents can be anywhere. The disk that comes with RetroWrite II has a `DOCUMENTS` folder, where the Open and Save dialogs start the first time.

## Appendix C: Limits and Memory

RetroWrite II runs in an Apple IIe with 128K. It keeps your document in the auxiliary 64K and reads the less-used parts of the program from the hard disk.

|   | Limit |
| --- | --- |
| Length of a document | 30,460 characters (about 15 typed pages) |
| Open documents | 4 (in the Windows menu) |
| Width of the text | 6.4 inches; margins at least ½ inch apart |
| Tab stops | 8 (then every ½ inch) |
| Fonts and sizes | 3 fonts, 3 sizes, bold, italic, underline, superscript, subscript |
| Header and footer | 39 characters each |
| Find and Replace text | 30 characters |
| Glossary entry | about 190 characters |
| File names | 15 characters (ProDOS) |
| Files listed in a folder | 48 |
| ODT documents | up to 30,460 characters of text, as above; opening a long one takes a minute or more at 1 MHz |
| Undo | there is no Undo: save often, and save copies of important work |

**Speed.** On a 1 MHz Apple IIe, typing a letter takes about a quarter of a second to show and a new screen of text about three seconds. Typing never loses letters: if you type faster than the screen, RetroWrite II catches up when you pause. An accelerator card makes it much quicker.

## Appendix D: Messages

These are the messages RetroWrite II can show, in alphabetical order, and what to do about them.

| Message | What to do |
| --- | --- |
| A file with that name already exists. Replace it? | Click Replace to write over it, or Cancel and type another name. |
| A hidden document could not be read back. | A RW2.WINn file was damaged or removed. The window starts empty; open the document again. |
| A text file can’t keep fonts, sizes or styles. Save it as text anyway? | Click Save to lose the formatting in the text file, or Cancel and use Save As. |
| Can’t write a work file in the RetroWrite II folder (disk full?). | Make room on the disk that holds RETROWRITE, then open the ODT document again. |
| BASIC.SYSTEM was not found. | Copy BASIC.SYSTEM into the RETROWRITE folder or the top level of the disk. |
| Can’t load part of RetroWrite II (RW2.OVLn is missing). | A program file is missing from the RETROWRITE folder. Copy the folder again. |
| Can’t open the document / Can’t put the document away | The disk is full, write-protected or offline. ProDOS’s error is shown with it. |
| Names are 1-15 letters, digits or periods, starting with a letter. | Type a ProDOS name, such as LETTER.2. |
| No printer card was found in slot n. | Choose the right slot in Page Setup. |
| Not found. | Find didn’t find the text anywhere in the document. |
| RW2.LC or RW2.FONTS is missing from the RetroWrite II folder. | Copy the RETROWRITE folder again. |
| RetroWrite II needs an Apple IIe with 128K. | The computer needs 128K (an extended 80-column card). |
| Save changes to this document before closing it? | Save keeps your work; Don’t Save throws the changes away; Cancel goes back. |
| SPELL.DICT was not found / is damaged | Copy SPELL.DICT into the RETROWRITE folder again. |
| That disk is not a DOS 3.3 disk (or there is no disk in the drive). | Check the disk and the drive, then try again. |
| The document is full. | The document has 30,460 characters. Split it into two documents. |
| There is no GLOSSARY file in the RetroWrite II folder. | Choose Edit Glossary to make one. |
| There is no Super Serial Card in slot n. | Choose the right slot in Communications. |
| This document came from a text file, which can’t keep fonts and styles. | Click RetroWrite to save a RetroWrite document (with a new name), or Text to save plain text. |
| This document is too large for RetroWrite II. | The file has more than 30,460 characters. |
| This document is protected with a password. | Save it again without a password in your word processor. |
| This document was too long for RetroWrite II: the end was left out. | The ODT document has more than 30,460 characters of text. Split it into smaller documents on the other computer. |
| This is not an ODT document. / This ODT document is damaged. | The file ends in .ODT but is not a complete ODT (ZIP) file. Copy it again; save it again as ODT. |
| This ODT document is compressed in a way RetroWrite II can’t read. | Save it again with LibreOffice, OpenOffice or Word. |
| This is not a RetroWrite II document. | The file is another program’s document. Text files open directly. |
| /RAM contains files that RetroWrite II will erase. | Press N, copy the files from /RAM to a disk, and start again; or press Y to go on. |

## Appendix E: The Document Format

For programmers: how a RetroWrite II document is stored on disk. Documents are ProDOS files of type $50 with aux type $5257 (“RW”).

| Bytes | Contents |
| --- | --- |
| 0-3 | `RW2D` |
| 4 | version: 3 |
| 5-6, 7-8 | left and right margin, in screen pixels from the text’s left edge (80 per inch) |
| 9-24 | eight tab stops, the same units, 0 = none |
| 25-64, 65-104 | header and footer, ASCII, ending with 0 |
| 105 - | the text |

The text is ASCII with Return ($0D) between paragraphs and Tab ($09). Bytes $80 and above are format marks that apply to the text after them:

| Byte | Meaning |
| --- | --- |
| $80 + f | character format f: bits 0-1 font (0 Sans, 1 Serif, 2 Mono), bits 2-3 size (0 9pt, 1 18pt, 2 27pt, 3 superscript; with bold set, subscript), bit 4 bold, bit 5 italic, bit 6 underline |
| $83 + p | paragraph mark at the start of a paragraph: bits 2-3 alignment (left, center, right, justify), bits 4-5 spacing (single, 1½, double) |
