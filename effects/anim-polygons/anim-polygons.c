#include "2d.h"
#include "blitter.h"
#include "cia.h"
#include "copper.h"
#include "effect.h"
#include "fx.h"
#include "line.h"
#include "memory.h"
#include "pixmap.h"
#include "types.h"

/* different line drawing methods */
#define CPULINE 0
#define CPUEDGE 0
#define BLITLINE 1

#define WIDTH 320
#define HEIGHT 180
#define DEPTH 4

#define HEIGHT_P1 210

static BitmapT *screen;
static CopInsT *bplptr[DEPTH];
static CopListT *cp;
static short active = 0;

#include "anim_data.c"
#include "data/anim-pal.c"

/* Reading polygon data */
static short current_frame = 0;
static short frame_count = 260 - 5;
static int vert_ptr = 0;
/* Synchronization */
int frame_diff = 0;
int frame_sync;

static void Load(void) {
    screen = NewBitmap(WIDTH, HEIGHT, DEPTH + 1);
    frame_sync = ReadFrameCounter();
}

static void UnLoad(void) { DeleteBitmap(screen); }

static void MakeCopperList(CopListT *cp) {
    CopInit(cp);
    CopSetupBitplanes(cp, bplptr, screen, DEPTH);
    {
        short *pixels = gradient.pixels;
        short i, j;

        for (i = 0; i < HEIGHT_P1 / 10; i++) {
            CopWait(cp, Y(i * 10 - 1), 0xde);
            for (j = 0; j < 16; j++) CopSetColor(cp, j, *pixels++);
        }
    }
    CopEnd(cp);
}

static void Init(void) {
    EnableDMA(DMAF_BLITTER);
    BitmapClear(screen);

    SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
    cp = NewCopList(100 + gradient.height * (gradient.width + 1));
    MakeCopperList(cp);
    CopListActivate(cp);
    EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
    DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);
    DeleteCopList(cp);
}

/* Get x on screen position from linear memory repr */
static inline short CalculateX(u_short xy) {
    return (short)(xy - (xy / WIDTH) * WIDTH);
}

/* Get y on screen position from linear memory repr */
static inline short CalculateY(u_short xy) { return (short)(xy / WIDTH); }

/* Using wrapper because it looked really messy with all those
 * #ifs in DrawSpans. In production it won't be necessary */
static void DrawLine(short xp, short yp, short xe, short ye) {
#if CPULINE == 1
    CpuLine(xp, yp, xe, ye);
#elif CPUEDGE == 1
    CpuEdge(xp, yp, xe, ye);
#elif BLITLINE == 1
    BlitterLine(xp, yp, xe, ye);
#endif
}

static void DrawFrame(void) {
    /* (x/y) previous */
    short xp = 0;
    short yp = 0;
    /* (x/y) end */
    short ye = 0;
    short xe = 0;
    /* (x/y) first in line strip */
    short xf = 0;
    short yf = 0;
    /* -- */
    u_char i = 0;
    u_short xy = 0;
    bool shape_start = true;

    WaitBlitter();

    i = verts_in_frame[current_frame];
    while (--i) {
        if (shape_start) {
            /* first vert in line strip */
            xy = verts[vert_ptr];
            xp = CalculateX(xy);
            yp = CalculateY(xy);
            xf = xp;
            yf = yp;
            vert_ptr++;
            shape_start = false;
        }
        xy = verts[vert_ptr];
        if (xy == 65535) {
            /* no more verts in line strip */
            DrawLine(xp, yp, xf, yf);
            vert_ptr++;
            shape_start = true;
            continue;
        }
        xe = CalculateX(xy);
        ye = CalculateY(xy);
        DrawLine(xp, yp, xe, ye);
        xp = xe;
        yp = ye;
        vert_ptr++;
    }

    current_frame++;

    if (current_frame > frame_count) {
        current_frame = 0;
        vert_ptr = 0;
    }
}

PROFILE(AnimRender);

static void Render(void) {
    ProfilerStart(AnimRender);
    {
        BlitterClear(screen, active);
#if CPULINE == 1
        CpuLineSetup(screen, active);
#elif CPUEDGE == 1
        CpuEdgeSetup(screen, active);
#elif BLITLINE == 1
        BlitterLineSetup(screen, active, LINE_EOR | LINE_ONEDOT);
#endif
        DrawFrame();
        BlitterFill(screen, active);
    }
    ProfilerStop(AnimRender);

    {
        short n = DEPTH;

        while (--n >= 0) {
            short i = (active + n + 1 - DEPTH) % (DEPTH + 1);
            if (i < 0) i += DEPTH + 1;
            CopInsSet32(bplptr[n], screen->planes[i]);
        }
    }

    /* synchronizing to frame counter */
    frame_diff = ReadFrameCounter() - frame_sync;
    if (frame_diff < 1) {
        TaskWaitVBlank();
    }
    TaskWaitVBlank();

    active = (active + 1) % (DEPTH + 1);
    frame_sync = ReadFrameCounter();
}

EFFECT(anim - lines, Load, UnLoad, Init, Kill, Render);

