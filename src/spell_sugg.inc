/* suggestions */
#define MAXSUG 9
#pragma bss-name (push, "LOWBSS")
static char sugg[MAXSUG][32];
static unsigned char cand[34];
#pragma bss-name (pop)
static unsigned char nsugg;
static unsigned char candl;

static void try_cand(void)
{
    unsigned char i;
    if (nsugg >= MAXSUG || candl < 2 || candl > 31) return;
    for (i = 0; i < nsugg; ++i)
        if (strlen(sugg[i]) == candl && !memcmp(sugg[i], cand, candl)) return;
    if (pool_find(cand, candl) || dict_find(cand, candl)) {
        memcpy(sugg[nsugg], cand, candl);
        sugg[nsugg][candl] = 0;
        ++nsugg;
    }
}

static void suggest(void)
{
    unsigned char n = swl, i, c;
    nsugg = 0;
    if (n > 24) return;
    /* transpositions */
    for (i = 0; i + 1 < n; ++i) {
        memcpy(cand, sw, n);
        c = cand[i]; cand[i] = cand[i + 1]; cand[i + 1] = c;
        candl = n;
        try_cand();
    }
    /* deletions */
    for (i = 0; i < n; ++i) {
        memcpy(cand, sw, i);
        memcpy(cand + i, sw + i + 1, n - i - 1);
        candl = n - 1;
        try_cand();
    }
    /* substitutions and insertions at position i, in dictionary order so
       that each dictionary block is read only once */
    /* deepest positions first (cheap: same few blocks); the second letter
       (position 1) spans many blocks, so only try it if still short */
    for (i = n; i >= 1 && nsugg < MAXSUG; --i) {
        if (i == 1 && nsugg >= 3) break;
        memcpy(cand, sw, i);
        for (c = '\''; c <= 'z'; ++c) {
            if (c == '\'' + 1) c = 'a';
            cand[i] = c;
            if (i < n && c != sw[i] && c != '\'') {
                memcpy(cand + i + 1, sw + i + 1, n - i - 1);
                candl = n;
                try_cand();
            }
            memcpy(cand + i + 1, sw + i, n - i);
            candl = n + 1;
            try_cand();
        }
        if ((PEEK(KBD) & 0xFF) == (0x80 | K_ESC)) return;
    }
    /* first-letter substitutions as a last resort */
    if (nsugg < 3) {
        memcpy(cand, sw, n);
        candl = n;
        for (c = 'a'; c <= 'z'; ++c) {
            if (c == sw[0]) continue;
            cand[0] = c;
            try_cand();
        }
    }
}

