#include <custom.h>
#include <effect.h>
#include <color.h>
#include <palette.h>
#include <sync.h>
#include <sprite.h>
#include <uae.h>
#include <system/task.h>
#include <system/interrupt.h>
#include <system/amigahunk.h>
#include <system/file.h>

#define _SYSTEM
#include <system/boot.h>
#include <system/cia.h>
#include <system/floppy.h>
#include <system/filesys.h>
#include <system/memfile.h>
#include <system/memory.h>

#include "demo.c"

static void ShowMemStats(void) {
  __unused unsigned chip_largest = MemAvail(MEMF_CHIP | MEMF_LARGEST);
  __unused unsigned fast_largest = MemAvail(MEMF_FAST | MEMF_LARGEST);
  __unused unsigned chip_total = MemAvail(MEMF_CHIP);
  __unused unsigned fast_total = MemAvail(MEMF_FAST);

  Log("[Memory] Max/total free: CHIP %6d/%6d  FAST %6d/%6d\n", chip_largest,
      chip_total, fast_largest, fast_total);
}

#define EXE_LOADER 0
#define EXE_SPOOKYTREE 1
#define EXE_DARKROOM 2
#define EXE_PROTRACKER 13
#define EXE_LAST 14

typedef struct ExeFile {
  const char *path;
  HunkT *hunk;
  EffectT *effect;
} ExeFileT;

#define EXEFILE(NUM, PATH) [NUM] = { .path = PATH, .hunk = NULL, .effect = NULL }

static __code ExeFileT ExeFile[EXE_LAST] = {
  EXEFILE(EXE_LOADER, "loader.exe"),
  EXEFILE(EXE_SPOOKYTREE, "spooky-tree.exe"),
  EXEFILE(EXE_DARKROOM, "darkroom.exe"),
  EXEFILE(EXE_PROTRACKER, "playpt.exe"),
};

static EffectT *LoadExe(int num) {
  ExeFileT *exe = &ExeFile[num];
  FileT *file;
  HunkT *hunk;
  u_char *ptr;

  Log("[Effect] Downloading '%s'\n", exe->path);

  file = OpenFile(exe->path);
  hunk = LoadHunkList(file);
  FileClose(file);

  if (hunk == NULL) {
    Panic("[Effect] Failed to load '%s'!", exe->path);
  }

  /* Assume code section is first and effect definition is at its end.
   * That should be the case as the effect definition is always the last in
   * source file. */
  ptr = &hunk->data[hunk->size - sizeof(EffectT)];
  while ((u_char *)ptr >= hunk->data) {
    if (*(u_int *)ptr == EFFECT_MAGIC) {
      EffectT *effect = (EffectT *)ptr;
      EffectLoad(effect);
      ShowMemStats();
      exe->effect = effect;
      exe->hunk = hunk;
      Log("[Effect] Effect '%s' is ready\n", effect->name);
      return effect;
    }
    ptr -= 2;
  }
  Panic("[Effect] '%s' missing effect magic marker!", exe->path);
}

static void UnLoadExe(int num) {
  ExeFileT *exe = &ExeFile[num];

  Log("[Effect] Removing '%s'\n", exe->path);

  EffectUnLoad(exe->effect);
  exe->effect = NULL;
  FreeHunkList(exe->hunk);
  exe->hunk = NULL;
}

static volatile EffectFuncT VBlankHandler = NULL;

static int VBlankISR(void) {
  if (VBlankHandler)
    VBlankHandler();
  return 0;
}

INTSERVER(VBlankInterrupt, 0, (IntFuncT)VBlankISR, NULL);

typedef enum {
  BG_IDLE = 0,
  BG_INIT = 1,
  BG_DEMO = 2,
} BgTaskStateT;

static __code volatile BgTaskStateT BgTaskState = BG_IDLE;

extern void CheckTrackmo(void);

static void BgTaskLoop(__unused void *ptr) {
  Log("[BgTask] Started!\n");

  for (;;) {
    switch (BgTaskState) {
      case BG_INIT:
        LoadExe(EXE_PROTRACKER);
        LoadExe(EXE_SPOOKYTREE);

        Log("[BgTask] Done initial loading!\n");
        BgTaskState = BG_IDLE;
        break;

      case BG_DEMO:
        {
          short cmd = TrackValueGet(&EffectLoader, ReadFrameCounter());
          if (cmd == 0)
            continue;

          Log("[BgTask] Got command %d!\n", cmd);

          if (cmd > 0) {
            /* load effect */
            LoadExe(cmd);
          } else {
            /* unload effect */
            cmd = -cmd;

            while (EffectIsRunning(ExeFile[cmd].effect));

            UnLoadExe(cmd);
          }
        }
        break;

      default:
        break;
    }
  }
}

static __aligned(8) char BgTaskStack[768];
static TaskT BgTask;

static void RunLoader(void) {
  EffectT *Loader = LoadExe(EXE_LOADER);

  EffectInit(Loader);
  VBlankHandler = Loader->VBlank;

  SetFrameCounter(0);
  lastFrameCount = 0;
  frameCount = 0;
  BgTaskState = BG_INIT;

  while (BgTaskState != BG_IDLE) {
    frameCount = ReadFrameCount();
    if (lastFrameCount != frameCount) {
      Loader->Render();
      TaskWaitVBlank();
    }
    lastFrameCount = frameCount;
  }

  VBlankHandler = NULL;
  EffectKill(Loader);
  UnLoadExe(EXE_LOADER);
}

static void RunEffects(void) {
  SetFrameCounter(0);
  lastFrameCount = 0;
  frameCount = 0;
  BgTaskState = BG_DEMO;

  for (;;) {
    static short prev = -1;
    short curr = TrackValueGet(&EffectNumber, frameCount);

    // Log("prev: %d, curr: %d, frameCount: %d\n", prev, curr, frameCount);

    if (prev != curr) {
      if (prev >= 0) {
        VBlankHandler = NULL;
        EffectKill(ExeFile[prev].effect);
      }
      if (curr == -1)
        break;
      if (ExeFile[curr].effect == NULL) {
        Panic("[Effect] '%s' did not load in time!", ExeFile[curr].path);
      }
      EffectInit(ExeFile[curr].effect);
      VBlankHandler = ExeFile[curr].effect->VBlank;
      ShowMemStats();
      Log("[Effect] Transition to %s took %d frames!\n",
          ExeFile[curr].effect->name, ReadFrameCounter() - lastFrameCount);
      lastFrameCount = ReadFrameCounter() - 1;
    }

    {
      EffectT *effect = ExeFile[curr].effect;
      short t = ReadFrameCount();
      frameCount = t;
      if ((lastFrameCount != frameCount) && effect->Render)
        effect->Render();
      lastFrameCount = t;
    }

    prev = curr;
  }
}

int main(void) {
  /* NOP that triggers fs-uae debugger to stop and inform GDB that it should
   * fetch segments locations to relocate symbol information read from file. */
  asm volatile("exg %d7,%d7");

  ResetSprites();
  AddIntServer(INTB_VERTB, VBlankInterrupt);

  /* Background thread may use tracks as well. */
  TrackInit(&EffectNumber);
  TrackInit(&EffectLoader);

  TaskInit(&BgTask, "background", BgTaskStack, sizeof(BgTaskStack));
  TaskRun(&BgTask, 1, BgTaskLoop, NULL);

  if (RightMouseButton()) {
    CheckTrackmo();
  }

  RunLoader();

  EffectInit(ExeFile[EXE_PROTRACKER].effect);
  UaeWarpMode(0);
  RunEffects();
  EffectKill(ExeFile[EXE_PROTRACKER].effect);

  RemIntServer(INTB_VERTB, VBlankInterrupt);

  /* Inspect the output to find memory leaks.
   * All memory should be released at this point! */
  {
    int i;

    for (i = 0; i < EXE_LAST; i++) {
      ExeFileT *exe = &ExeFile[i];
      if (exe->hunk) {
        Log("[Effect] Effect '%s' has not been removed from memory!\n", exe->path);
        UnLoadExe(i);
      }
    }
  }

  MemCheck(1);

  return 0;
}
