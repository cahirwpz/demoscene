#include "effect.h"

/* Search following header files for useful procedures. */
#include "blitter.h" /* blitter handling routines */
#include "copper.h"  /* copper list construction */
#include "bitmap.h"  /* bitmap structure */
#include "palette.h" /* palette structure */
#include "circle.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5
#define CENTER_X (WIDTH / 2)
#define CENTER_Y (HEIGHT / 2 + 25)
#define SMALL_R 20
#define FIRST_LENS 3
#define SECOND_LENS 4

#include "data/clock-bg.c"
#include "data/clock-palette.c"
#include "data/movement_data.c"

static CopListT *cp;
static BitmapT *bckg;
int old_x[2], old_y[2];

static void Circles(int intra_x, int intra_y, int progress, int smallR)
{
    int by_two = progress & 1;
    int plane = by_two ? FIRST_LENS : SECOND_LENS;

    int x = circle_movements[by_two].x_cos[progress] + intra_x;
    int y = circle_movements[by_two].y_sin[progress] + intra_y;

    {
        Area2D area = {.x = old_x[by_two] - smallR - 2, .y = old_y[by_two] - smallR - 2, .w = smallR * 2 + 4, .h = smallR * 2 + 4};
        BlitterClearArea(bckg, plane, &area);
    }

    CircleEdge(bckg, plane, x, y, smallR);
    BlitterFill(bckg, plane);

    old_x[by_two] = x;
    old_y[by_two] = y;
}

static void SetupBoxes(void)
{
    // Top colour bar
    BlitterLineSetup(bckg, 3, LINE_OR | LINE_SOLID);
    BlitterLine(253, 8, 253, 43);
    BlitterLine(60, 8, 10, 60);
    BlitterLineSetup(bckg, 4, LINE_OR | LINE_SOLID);
    BlitterLine(253, 8, 253, 43);
    BlitterLine(60, 8, 10, 60);

    // Left colour bar
    BlitterLineSetup(bckg, 3, LINE_OR | LINE_SOLID);
    BlitterLine(50, 65, 50, 195);
    BlitterLine(7, 65, 7, 195);

    // Right colour bar
    BlitterLineSetup(bckg, 4, LINE_OR | LINE_SOLID);
    BlitterLine(305, 65, 305, 195);
    BlitterLine(260, 65, 260, 195);
}

static void Init(void)
{
    bckg = NewBitmap(WIDTH, HEIGHT, DEPTH);

    SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
    LoadPalette(&clock_pal, 0);

    EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);
    BitmapCopy(bckg, 0, 0, &clock_bg);

    cp = NewCopList(80);
    CopInit(cp);
    CopSetupBitplanes(cp, NULL, bckg, DEPTH);
    CopEnd(cp);
    CopListActivate(cp);

    BlitterClear(bckg, 3);
    BlitterClear(bckg, 4);

    SetupBoxes();

    EnableDMA(DMAF_RASTER);

    old_x[0] = circle_movements[0].x_cos[0] + CENTER_X;
    old_y[0] = circle_movements[0].y_sin[0] + CENTER_Y;
    old_x[1] = circle_movements[1].x_cos[1] + CENTER_X;
    old_y[1] = circle_movements[1].y_sin[1] + CENTER_Y;
}

static void Kill(void)
{
    DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);
    DeleteCopList(cp);
    DeleteBitmap(bckg);
}

static void Render(void)
{
    static int progress = 0;
    Circles(CENTER_X, CENTER_Y, progress, SMALL_R);
    progress++;
    progress &= 255;
    TaskWaitVBlank();
}

EFFECT(letter_clock, NULL, NULL, Init, Kill, Render);