// gbashot: boot a GBA ROM headlessly with mGBA's core, play a scripted input
// sequence and save PNG screenshots. Dev tool for Pokémon Ascension (no display needed).
//
// Usage: gbashot ROM SCRIPT OUTDIR
// SCRIPT lines (one per line, # comments):
//   wait N              run N frames with no keys held
//   press KEYS [N]      hold KEYS (e.g. A, START, L+SELECT) for N frames (default 6), then release
//   hold KEYS N         same as press, explicit duration
//   shot NAME           save OUTDIR/NAME.png (240x160)
// Keys: A B SELECT START RIGHT LEFT UP DOWN R L
#include <mgba/core/core.h>
#include <mgba/core/config.h>
#include <mgba/core/log.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Quiet by default; GBASHOT_GAME_LOG=1 prints the game's own debug output (mGBA "GBA Debug"
// category, e.g. the Battle Lab turn log) to stderr.
static void quiet_log(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a)
{
    (void) l; (void) lv;
    if (getenv("GBASHOT_GAME_LOG") && strcmp(mLogCategoryName(c), "GBA Debug") == 0)
    {
        vfprintf(stderr, f, a);
        fputc('\n', stderr);
    }
}
static struct mLogger sQuiet = { .log = quiet_log };

static int key_bits(const char *spec) {
    static const char *names[] = {"A", "B", "SELECT", "START", "RIGHT", "LEFT", "UP", "DOWN", "R", "L"};
    int bits = 0; char buf[128]; strncpy(buf, spec, sizeof buf - 1); buf[sizeof buf - 1] = 0;
    for (char *tok = strtok(buf, "+"); tok; tok = strtok(NULL, "+")) {
        int found = 0;
        for (int i = 0; i < 10; i++) if (!strcasecmp(tok, names[i])) { bits |= 1 << i; found = 1; }
        if (!found) { fprintf(stderr, "unknown key: %s\n", tok); exit(2); }
    }
    return bits;
}

static void run(struct mCore *core, int keys, int frames) {
    core->setKeys(core, keys);
    for (int i = 0; i < frames; i++) core->runFrame(core);
    core->setKeys(core, 0);
}

static int save_png(const char *path, const color_t *buf, unsigned w, unsigned h, size_t stride) {
    FILE *f = fopen(path, "wb"); if (!f) return -1;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png))) { fclose(f); return -1; }
    png_init_io(png, f);
    png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    unsigned char *row = malloc(w * 3);
    for (unsigned y = 0; y < h; y++) {
        for (unsigned x = 0; x < w; x++) {
            uint32_t c = buf[y * stride + x];          // mGBA 32-bit colour: 0x00BBGGRR
            row[x * 3] = c & 0xFF; row[x * 3 + 1] = (c >> 8) & 0xFF; row[x * 3 + 2] = (c >> 16) & 0xFF;
        }
        png_write_row(png, row);
    }
    png_write_end(png, NULL); png_destroy_write_struct(&png, &info); free(row); fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) { fprintf(stderr, "usage: %s ROM SCRIPT OUTDIR\n", argv[0]); return 2; }
    mLogSetDefaultLogger(&sQuiet);
    struct mCore *core = mCoreFind(argv[1]);
    if (!core || !core->init(core)) { fprintf(stderr, "cannot open %s\n", argv[1]); return 1; }
    mCoreInitConfig(core, NULL);
    unsigned w, h; core->desiredVideoDimensions(core, &w, &h);
    color_t *video = calloc((size_t) w * h, sizeof(color_t));
    core->setVideoBuffer(core, video, w);
    if (!mCoreLoadFile(core, argv[1])) { fprintf(stderr, "cannot load %s\n", argv[1]); return 1; }
    core->reset(core);

    FILE *s = fopen(argv[2], "r"); if (!s) { perror(argv[2]); return 1; }
    char line[256]; int n = 0;
    while (fgets(line, sizeof line, s)) {
        char cmd[32], arg[128]; int frames = 0;
        if (line[0] == '#' || sscanf(line, "%31s", cmd) != 1) continue;
        if (!strcmp(cmd, "wait") && sscanf(line, "%*s %d", &frames) == 1) run(core, 0, frames);
        else if ((!strcmp(cmd, "press") || !strcmp(cmd, "hold")) && sscanf(line, "%*s %127s %d", arg, &frames) >= 1) { run(core, key_bits(arg), frames > 0 ? frames : 6); run(core, 0, 2); }
        else if (!strcmp(cmd, "shot") && sscanf(line, "%*s %127s", arg) == 1) {
            char path[512]; snprintf(path, sizeof path, "%s/%s.png", argv[3], arg);
            if (save_png(path, video, w, h, w)) { fprintf(stderr, "cannot write %s\n", path); return 1; }
            printf("%s\n", path); n++;
        } else { fprintf(stderr, "bad script line: %s", line); return 2; }
    }
    fclose(s); core->deinit(core); free(video);
    return n > 0 ? 0 : 1;
}
