/* aux language card slots (512 bytes each): 31 slots */
#define NSLOT 31
static unsigned slot_addr(unsigned char s, unsigned char *mode)
{
    if (s < 8)  { *mode = 0; return 0xD000 + (s << 9); }
    if (s < 23) { *mode = 0; return 0xE000 + ((s - 8) << 9); }
    *mode = 2;
    return 0xD000 + ((s - 23) << 9);
}

static void slot_get(unsigned char s, void *buf)
{
    unsigned char m;
    unsigned a = slot_addr(s, &m);
    alc_xfer(buf, a, m);
}

static void slot_put(unsigned char s, void *buf)
{
    unsigned char m;
    unsigned a = slot_addr(s, &m);
    alc_xfer(buf, a, m | 1);
}

/* the three 512-byte dictionary buffers borrow the line buffer ($1400);
   any drawing destroys them, so dict_stale() is called after drawing */
#define dtop ((unsigned char *)0x1400)      /* block 0 */
#define dbuf ((unsigned char *)0x1600)      /* current directory block */
#define bbuf ((unsigned char *)0x1800)      /* current data block (asm: _bbuf) */
static unsigned char dtop_ok;
static unsigned char ndir;
static unsigned ndata;
static unsigned char dir_cached;    /* directory blocks in aux LC */
static int dfd = -1;
static unsigned char cur_dir = 0xFF;
static unsigned cur_blk = 0xFFFF;
static unsigned char dslot0;        /* first data cache slot */
static unsigned cblk[NSLOT];        /* data block in each cache slot */
#pragma bss-name (push, "LOWBSS")    /* $0200-$03CF, see rw2gfx.cfg */
static unsigned char cage[NSLOT];
#pragma bss-name (pop)
static unsigned char clock_;
static unsigned char dict_ready;
#pragma bss-name (push, "LOWBSS")
static unsigned char lt[32];
#pragma bss-name (pop)
static unsigned char ltl;   /* previous target in current block */
static unsigned char lt_valid;
static unsigned char spell_err;

/* block range of current data block, to skip index searches */
#pragma bss-name (push, "LOWBSS")
static unsigned char blo[32], bhi[32];
#pragma bss-name (pop)
static unsigned char blol, bhil, bhi_valid;

/* the learned/ignored word pool is resident (core.c), so it survives
   while other overlays are loaded:  [len|0x80 ignore][chars]... 0 */
extern unsigned char upool_loaded;
#define upool ((unsigned char *)0x0C00)  /* $0C00-$0FFF (no second ProDOS buffer) */
extern unsigned upool_len;

static unsigned char read_dict_block(unsigned blk, void *buf)
{
    if (lseek(dfd, (long)blk << 9, SEEK_SET) < 0) return 0;
    if (read(dfd, buf, 512) != 512) return 0;
    return 1;
}

static void dict_path(char *out, const char *dir, const char *file)
{
    strcpy(out, dir);
    strcat(out, "/");
    strcat(out, file);
}

static unsigned char dict_open(void)
{
    unsigned char i;
    if (dict_ready) return 1;
    dict_path(tmppath, progdir, "SPELL.DICT");
    dfd = open(tmppath, O_RDONLY);
    if (dfd < 0) {
        show_msg("SPELL.DICT was not found in the\nRetroWrite II folder.");
        return 0;
    }
    if (!read_dict_block(0, dtop) || memcmp(dtop, "SPL1", 4)) {
        close(dfd);
        dfd = -1;
        show_msg("SPELL.DICT is damaged or in the\nwrong format.");
        return 0;
    }
    dtop_ok = 1;
    ndir = dtop[4];
    ndata = dtop[5] | (dtop[6] << 8);
    dir_cached = 0;
    dslot0 = 0;
    if (ndir <= NSLOT - 4) {
        for (i = 0; i < ndir; ++i) {
            if (!read_dict_block(1 + i, iob)) break;
            slot_put(i, iob);
        }
        if (i == ndir) {
            dir_cached = 1;
            dslot0 = ndir;
        }
    }
    for (i = 0; i < NSLOT; ++i) { cblk[i] = 0xFFFF; cage[i] = 0; }
    cur_dir = 0xFF;
    cur_blk = 0xFFFF;
    bhi_valid = 0;
    blol = 0;
    dict_ready = 1;
    return 1;
}

static unsigned rdbase;
static void dict_stale(void)
{
    cur_dir = 0xFF;
    cur_blk = 0xFFFF;
    lt_valid = 0;
    blol = 0;
    bhi_valid = 0;
    dtop_ok = 0;
    rdbase = 0xFFFF;
}

static void dict_close(void)
{
    if (dfd >= 0) close(dfd);
    dfd = -1;
    dict_ready = 0;
}

static unsigned char load_dir_block(unsigned char d)
{
    if (cur_dir == d) return 1;
    if (dir_cached) slot_get(d, dbuf);
    else if (!read_dict_block(1 + d, dbuf)) { spell_err = 1; return 0; }
    cur_dir = d;
    return 1;
}

static unsigned char load_data_block(unsigned b)
{
    unsigned char i, best;
    if (cur_blk == b) return 1;
    for (i = dslot0; i < NSLOT; ++i) {
        if (cblk[i] == b) {
            lt_valid = 0;
            slot_get(i, bbuf);
            cage[i] = ++clock_;
            cur_blk = b;
            return 1;
        }
    }
    lt_valid = 0;
    if (!read_dict_block(1 + ndir + b, bbuf)) { spell_err = 1; return 0; }
    /* replace least recently used slot */
    best = dslot0;
    for (i = dslot0; i < NSLOT; ++i) {
        if (cblk[i] == 0xFFFF) { best = i; break; }
        if ((unsigned char)(clock_ - cage[i]) > (unsigned char)(clock_ - cage[best])) best = i;
    }
    if (best < NSLOT) {
        slot_put(best, bbuf);
        cblk[best] = b;
        cage[best] = ++clock_;
    }
    cur_blk = b;
    return 1;
}

extern unsigned char scan_resume, sm;

/* scan current block, resuming from the previous lookup when possible */
static unsigned char scan_block(const unsigned char *w, unsigned char wl)
{
    unsigned char k, r;
    scan_resume = 0;
    if (lt_valid && cmpw(lt, ltl, w, wl) < 0) {
        for (k = 0; k < ltl && k < wl && lt[k] == w[k]; ++k) ;
        if (sm > k) sm = k;
        scan_resume = 1;
    }
    r = scan_blk(w, wl);
    memcpy(lt, w, wl);
    ltl = wl;
    lt_valid = 1;
    return r;
}

static unsigned char dict_find(const unsigned char *w, unsigned char wl)
{
    unsigned p, lastp;
    signed char d, k;
    unsigned char idx, l, nk;
    unsigned first;

    /* fast path: target inside the current data block's range */
    if (cur_blk != 0xFFFF && blol && cmpw(blo, blol, w, wl) <= 0 &&
        (!bhi_valid || cmpw(w, wl, bhi, bhil) < 0))
        return scan_block(w, wl);

    /* top index */
    if (!dtop_ok) {
        if (!read_dict_block(0, dtop)) { spell_err = 1; return 0; }
        dtop_ok = 1;
    }
    p = 7;
    d = -1;
    idx = 0;
    while ((l = dtop[p]) != 0) {
        if (cmpw(dtop + p + 1, l, w, wl) > 0) break;
        d = idx++;
        p += 1 + l;
    }
    if (d < 0) return 0;
    if (!load_dir_block(d)) return 0;
    first = dbuf[0] | (dbuf[1] << 8);
    p = 2;
    k = -1;
    idx = 0;
    lastp = 0;
    while ((l = dbuf[p]) != 0) {
        if (cmpw(dbuf + p + 1, l, w, wl) > 0) break;
        k = idx++;
        lastp = p;
        p += 1 + l;
    }
    if (k < 0) return 0;
    blol = dbuf[lastp];
    memcpy(blo, dbuf + lastp + 1, blol);
    /* upper bound: next entry in this dir block, or next top entry */
    nk = dbuf[p];
    if (nk) {
        memcpy(bhi, dbuf + p + 1, nk);
        bhil = nk;
        bhi_valid = 1;
    } else {
        bhi_valid = 0;
        /* fall back: next top index entry if any */
        p = 7;
        idx = 0;
        while ((l = dtop[p]) != 0) {
            if (idx == (unsigned char)d + 1) {
                memcpy(bhi, dtop + p + 1, l);
                bhil = l;
                bhi_valid = 1;
                break;
            }
            ++idx;
            p += 1 + l;
        }
        if (!bhi_valid && !dtop[p]) { bhi_valid = 0; }
    }
    if (!load_data_block(first + k)) { blol = 0; return 0; }
    return scan_block(w, wl);
}

/* user dictionary + ignore list */
static unsigned char pool_find(const unsigned char *w, unsigned char wl)
{
    unsigned p = 0;
    unsigned char l;
    while (p < upool_len && (l = upool[p]) != 0) {
        if ((l & 0x7F) == wl && !memcmp(upool + p + 1, w, wl)) return 1;
        p += 1 + (l & 0x7F);
    }
    return 0;
}

static unsigned char pool_add(const unsigned char *w, unsigned char wl, unsigned char ignore)
{
    if (upool_len + wl + 2 > UPOOL) return 0;
    upool[upool_len] = wl | (ignore ? 0x80 : 0);
    memcpy(upool + upool_len + 1, w, wl);
    upool_len += wl + 1;
    upool[upool_len] = 0;
    return 1;
}

static void user_load(void)
{
    int fd, n, i;
    unsigned char c, wl = 0;
    static unsigned char w[32];
    if (upool_loaded) return;
    upool_loaded = 1;
    upool_len = 0;
    upool[0] = 0;
    dict_path(tmppath, progdir, "USER.DICT");
    fd = open(tmppath, O_RDONLY);
    if (fd < 0) return;
    while ((n = read(fd, iob, 512)) > 0) {
        for (i = 0; i < n; ++i) {
            c = lowc(iob[i] & 0x7F);
            if ((c >= 'a' && c <= 'z') || c == '\'') {
                if (wl < 31) w[wl++] = c;
            } else {
                if (wl >= 1) pool_add(w, wl, 0);
                wl = 0;
            }
        }
    }
    if (wl) pool_add(w, wl, 0);
    close(fd);
}

static void dict_reopen(void)
{
    if (!dict_ready) return;
    dict_path(tmppath, progdir, "SPELL.DICT");
    dfd = open(tmppath, O_RDONLY);
    if (dfd < 0) spell_err = 1;
}

static unsigned char user_save(void)
{
    int fd;
    unsigned p = 0, n = 0;
    unsigned char l;
    if (dfd >= 0) close(dfd);           /* only one file may be open */
    dfd = -1;
    dict_path(tmppath, progdir, "USER.DICT");
    _filetype = PRODOS_T_TXT;
    _auxtype = 0;
    fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) { dict_reopen(); return 0; }
    while ((l = upool[p]) != 0) {
        if (!(l & 0x80)) {
            if (n + l + 1 > 512) { write(fd, iob, n); n = 0; }
            memcpy(iob + n, upool + p + 1, l);
            n += l;
            iob[n++] = CR;
        }
        p += 1 + (l & 0x7F);
    }
    if (n) write(fd, iob, n);
    close(fd);
    dict_reopen();
    return 1;
}

