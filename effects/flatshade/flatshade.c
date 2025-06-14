#include "beampos.h"
#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "sprite.h"
#include "3d.h"
#include "fx.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH  4

static __code Object3D *cube;
static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code CopInsPairT *sprptr;
static __code BitmapT *screen[2];
static __code BitmapT *buffer;
static __code int active = 0;

#include "data/flatshade-pal.c"
#include "data/stripe-1.c"
#include "data/stripe-2.c"
#include "data/stripe-3.c"
#include "data/stripe-4.c"
#include "data/stripe-colors.c"
#include "data/codi.c"

static __code SpriteT *stripe[8] = {
  stripe_1_0,
  stripe_1_1,
  stripe_2_0,
  stripe_2_1,
  stripe_3_0,
  stripe_3_1,
  stripe_4_0,
  stripe_4_1,
};

typedef struct StripeBackup {
  u_int header;
  u_int footer;
  short offset;
} StripeBackupT;

static __code short offset[8];
static __code StripeBackupT stripeBackup[8];

static void Load(void) {
  short i, j;

  /* stripe 1-3 have 96 lines, and stripe 4 48 lines
   * we need to copy them to create 384 lines long sprites */

  memcpy(&stripe[6]->data[48], &stripe[6]->data[0], 48 * sizeof(SprDataT));
  memcpy(&stripe[7]->data[48], &stripe[7]->data[0], 48 * sizeof(SprDataT));

  for (i = 96; i < 384; i += 96) {
    for (j = 0; j < 8; j++) {
      memcpy(&stripe[j]->data[i],&stripe[j]->data[0], 96 * sizeof(SprDataT));
    }
  }
}

static void SaveStripes(short offset[8]) {
  short i;

  for (i = 0; i < 8; i++) {
    short off = *offset++;
    u_int *data = (u_int *)&stripe[i]->data[off];

    stripeBackup[i].header = data[-1];
    stripeBackup[i].footer = data[HEIGHT];
    stripeBackup[i].offset = off;
  }
}

static void RestoreStripes(void) {
  short i;

  for (i = 0; i < 8; i++) {
    short off = stripeBackup[i].offset;
    u_int *data = (u_int *)&stripe[i]->data[off];

    data[-1] = stripeBackup[i].header;
    data[HEIGHT] = stripeBackup[i].footer;
  }
}

static void UpdateStripes(short offset[8]) {
  short i;

  RestoreStripes();
  SaveStripes(offset);

  for (i = 0; i < 8; i++) {
    short off = *offset++;
    SpriteT *spr = (SpriteT *)&stripe[i]->data[off - 1];
    spr->pos = SPRPOS(DIWHP + 32 + 16 * i, DIWVP);
    spr->ctl = SPRCTL(DIWHP + 32 + 16 * i, DIWVP, HEIGHT, false);
    CopInsSetSprite(&sprptr[i], spr);
  }
}

static CopListT *MakeCopperList(void) {
  CopListT *cp;
  u_short *pixels = gradient_pixels;
  short i, j;

  cp = NewCopList(64 + HEIGHT * (9 + 9 + 12));
  bplptr = CopSetupBitplanes(cp, screen[0], DEPTH);
  sprptr = CopSetupSprites(cp);

  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(i-1), HP(454 - 64));

    for (j = 0; j < 4; j++) {
      CopMove16(cp, spr[j*2+0].pos, SPRPOS(X(32 + 32*j).hpos, Y(i).vpos));
      CopMove16(cp, spr[j*2+1].pos, SPRPOS(X(32 + 32*j + 16).hpos, Y(i).vpos));
    }
    CopMove16(cp, color[17], *pixels++);
    CopMove16(cp, color[18], *pixels++);
    CopMove16(cp, color[19], *pixels++);
    CopMove16(cp, color[21], *pixels++);
    CopMove16(cp, color[22], *pixels++);
    CopMove16(cp, color[23], *pixels++);
    CopMove16(cp, color[25], *pixels++);
    CopMove16(cp, color[26], *pixels++);
    CopMove16(cp, color[27], *pixels++);
    CopMove16(cp, color[29], *pixels++);
    CopMove16(cp, color[30], *pixels++);
    CopMove16(cp, color[31], *pixels++);

    CopWait(cp, Y(i), X(128+8));
    for (j = 3; j >= 0; j--) {
      CopMove16(cp, spr[j*2+0].pos, SPRPOS(DIWHP + 256 - 32*j + 0, DIWVP + i));
      CopMove16(cp, spr[j*2+1].pos, SPRPOS(DIWHP + 256 - 32*j + 16, DIWVP + i));
    }
  }

  CopListFinish(cp);
  return cp;
}

static void Init(void) {
  cube = NewObject3D(&codi);
  cube->translate.z = fx4i(-300);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, 0);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, 0);
  buffer = NewBitmap(WIDTH, HEIGHT, 1, 0);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);
  BitmapClear(screen[0]);
  BitmapClear(screen[1]);
  BitmapClear(buffer);

  /* keep the buffer as the last bitplane of both screens */
  screen[0]->planes[DEPTH] = buffer->planes[0];
  screen[1]->planes[DEPTH] = buffer->planes[0];

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  LoadColors(flatshade_colors, 0);
  LoadColors(stripe_1_colors, 16);
  LoadColors(stripe_2_colors, 20);
  LoadColors(stripe_3_colors, 24);
  LoadColors(stripe_4_colors, 28);

  // Sprites are below foreground
  custom->bplcon2 = 0;

  cp = MakeCopperList();
  CopListActivate(cp);

  SaveStripes(offset);
  UpdateStripes(offset);

  EnableDMA(DMAF_RASTER | DMAF_SPRITE | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  CopperStop();
  ResetSprites();
  DisableDMA(DMAF_RASTER);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(buffer);
  DeleteCopList(cp);
  DeleteObject3D(cube);
}

#define MULVERTEX1(D, E) {                      \
  short t0 = (*v++) + y;                        \
  short t1 = (*v++) + x;                        \
  int t2 = (*v++) * z;                          \
  v++;                                          \
  D = ((t0 * t1 + t2 - xy) >> 4) + E;           \
}

#define MULVERTEX2(D) {                         \
  short t0 = (*v++) + y;                        \
  short t1 = (*v++) + x;                        \
  int t2 = (*v++) * z;                          \
  short t3 = (*v++);                            \
  D = normfx(t0 * t1 + t2 - xy) + t3;           \
}

static void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  void *_objdat = object->objdat;
  short *group = object->vertexGroups;

  int m0 = (M->x << 8) - ((M->m00 * M->m01) >> 4);
  int m1 = (M->y << 8) - ((M->m10 * M->m11) >> 4);

  /* WARNING! This modifies camera matrix! */
  M->z -= normfx(M->m20 * M->m21);

  /*
   * A = m00 * m01
   * B = m10 * m11
   * C = m20 * m21 
   * yx = y * x
   *
   * (m00 + y) * (m01 + x) + m02 * z - yx + (mx - A)
   * (m10 + y) * (m11 + x) + m12 * z - yx + (my - B)
   * (m20 + y) * (m21 + x) + m22 * z - yx + (mz - C)
   */

  do {
    short i;

    while ((i = *group++)) {
      if (NODE3D(i)->flags) {
        short *pt = (short *)NODE3D(i);
        short *v = (short *)M;
        short x, y, z, zp;
        int xy, xp, yp;

        /* clear flags */
        *pt++ = 0;

        x = *pt++;
        y = *pt++;
        z = *pt++;
        xy = x * y;

        MULVERTEX1(xp, m0);
        MULVERTEX1(yp, m1);
        MULVERTEX2(zp);

        *pt++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
        *pt++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
        *pt++ = zp;
      }
    }
  } while (*group);
}

static void DrawObject(Object3D *object, void **planes,
                       CustomPtrT custom_ asm("a6"))
{
  register SortItemT *item asm("a3") = object->visibleFace;
  void **scrbpl = &planes[DEPTH];
  void *_objdat = object->objdat;

  custom_->bltafwm = -1;
  custom_->bltalwm = -1;

  for (; item->index >= 0; item++) {
    short ii = item->index;

    {
      register short *index asm("a4") = (short *)&FACE(ii)->count;
      short m = (*index++) - 1;

      do {
        /* Draw edge. */
        short bltcon0, bltcon1, bltsize, bltbmod, bltamod;
        int bltapt, bltcpt;
        short x0, y0, x1, y1;
        short dmin, dmax, derr;

        /* skip vertex index */
        index++;

        {
          short i = *index++; /* edge index */
          short *edge = &EDGE(i)->point[0];
          short *vertex;

          vertex = &VERTEX(*edge++)->x;
          x0 = *vertex++;
          y0 = *vertex++;

          vertex = &VERTEX(*edge++)->x;
          x1 = *vertex++;
          y1 = *vertex++;
        }

        if (y0 == y1)
          continue;

        if (y0 > y1) {
          swapr(x0, x1);
          swapr(y0, y1);
        }

        dmax = x1 - x0;
        if (dmax < 0)
          dmax = -dmax;

        dmin = y1 - y0;
        if (dmax >= dmin) {
          if (x0 >= x1)
            bltcon1 = AUL | SUD | LINEMODE | ONEDOT;
          else
            bltcon1 = SUD | LINEMODE | ONEDOT;
        } else {
          if (x0 >= x1)
            bltcon1 = SUL | LINEMODE | ONEDOT;
          else
            bltcon1 = LINEMODE | ONEDOT;
          swapr(dmax, dmin);
        }

        bltcpt = (int)scrbpl[0] + (short)(((y0 << 5) + (x0 >> 3)) & ~1);

        bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_EOR;
        bltcon1 |= rorw(x0 & 15, 4);

        dmin <<= 1;
        derr = dmin - dmax;
        if (derr < 0)
          bltcon1 |= SIGNFLAG;

        bltamod = derr - dmax;
        bltbmod = dmin;
        bltsize = (dmax << 6) + 66;
        bltapt = derr;

        _WaitBlitter(custom_);

        custom_->bltbdat = 0xffff;
        custom_->bltadat = 0x8000;
        custom_->bltcon0 = bltcon0;
        custom_->bltcon1 = bltcon1;
        custom_->bltcpt = (void *)bltcpt;
        custom_->bltapt = (void *)bltapt;
        custom_->bltdpt = scrbpl[0];
        custom_->bltcmod = WIDTH / 8;
        custom_->bltbmod = bltbmod;
        custom_->bltamod = bltamod;
        custom_->bltdmod = WIDTH / 8;
        custom_->bltsize = bltsize;
      } while (--m != -1);
    }

    {
      short bltstart, bltend;
      short bltmod, bltsize;

      {
        short minX, minY, maxX, maxY;

        register short *index asm("a4") = (short *)&FACE(ii)->count;
        short m = (*index++) - 2;
        short *vertex;

        vertex = &VERTEX(*index++)->x;
        index++; /* skip edge index */

        minX = maxX = *vertex++;
        minY = maxY = *vertex++;

        /* Calculate area. */
        do {
          short x, y;

          vertex = &VERTEX(*index++)->x;
          index++; /* skip edge index */

          x = *vertex++;
          y = *vertex++;

          if (x < minX)
            minX = x;
          else if (x > maxX)
            maxX = x;
          if (y < minY)
            minY = y;
          else if (y > maxY)
            maxY = y;
        } while (--m != -1);

        /* Align to word boundary. */
        minX &= ~15;
        minX >>= 3;
        /* to avoid case where a line is on right edge */
        maxX += 16;
        maxX &= ~15;
        maxX >>= 3;

        {
          short w = maxX - minX;
          short h = maxY - minY + 1;

          bltstart = minX + minY * (WIDTH / 8);
          bltend = maxX + maxY * (WIDTH / 8) - 2;
          bltsize = (h << 6) | (w >> 1);
          bltmod = (WIDTH / 8) - w;
        }
      }

      /* Fill face. */
      {
        void *src = scrbpl[0] + bltend;

        _WaitBlitter(custom_);

        custom_->bltcon0 = (SRCA | DEST) | A_TO_D;
        custom_->bltcon1 = BLITREVERSE | FILL_XOR;
        custom_->bltapt = src;
        custom_->bltdpt = src;
        custom_->bltamod = bltmod;
        custom_->bltbmod = bltmod;
        custom_->bltdmod = bltmod;
        custom_->bltsize = bltsize;
      }

      /* Copy filled face to screen. */
      {
        void **dstbpl = scrbpl;
        void *src = scrbpl[0] + bltstart;
        char mask = 1 << (DEPTH - 1);
        char color = FACE(ii)->flags;

        do {
          void *dst = *(--dstbpl) + bltstart;
          u_short bltcon0;

          if (color & mask) {
            bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
          } else {
            bltcon0 = (SRCA | SRCB | DEST) | (NABC | NABNC);
          }

          _WaitBlitter(custom_);

          custom_->bltcon0 = bltcon0;
          custom_->bltcon1 = 0;
          custom_->bltapt = src;
          custom_->bltbpt = dst;
          custom_->bltdpt = dst;
          custom_->bltsize = bltsize;

          mask >>= 1;
        } while (mask);
      }

      /* Clear working area. */
      {
        void *data = scrbpl[0] + bltstart;

        _WaitBlitter(custom_);

        custom_->bltcon0 = (DEST | A_TO_D);
        custom_->bltadat = 0;
        custom_->bltdpt = data;
        custom_->bltsize = bltsize;
      }
    }
  }
}

static void BitmapClearFast(BitmapT *dst) {
  u_short height = (short)dst->height * (short)dst->depth;
  u_short bltsize = (height << 6) | (dst->bytesPerRow >> 1);
  void *bltpt = dst->planes[0];

  WaitBlitter();

  custom->bltcon0 = DEST | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0;
  custom->bltdmod = 0;
  custom->bltdpt = bltpt;
  custom->bltsize = bltsize;
}

PROFILE(Transform);
PROFILE(Draw);

static void Render(void) {
  BitmapClearFast(screen[active]);

  ProfilerStart(Transform);
  {
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 8;
    UpdateObjectTransformation(cube);
    UpdateFaceVisibility(cube);
    UpdateVertexVisibility(cube);
    TransformVertices(cube);
    SortFaces(cube);
  }
  ProfilerStop(Transform); // Average: 163

  ProfilerStart(Draw);
  {
    DrawObject(cube, screen[active]->planes, custom);
  }
  ProfilerStop(Draw); // Average: 671

  CopUpdateBitplanes(bplptr, screen[active], DEPTH);
  TaskWaitVBlank();
  active ^= 1;
}

static void VBlank(void) {
#if 0
  static short frameCount = 0;
  frameCount += 3;

  offset[0] = offset[1] = (frameCount) % 96;
  offset[2] = offset[3] = (frameCount * 7 / 8) % 96;
  offset[4] = offset[5] = (frameCount * 6 / 8) % 96;
  offset[6] = offset[7] = (frameCount * 5 / 8) % 48;
#endif

  UpdateStripes(offset);
}

EFFECT(FlatShade, Load, NULL, Init, Kill, Render, VBlank);
