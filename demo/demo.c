#include <clib/exec_protos.h>
#include <proto/exec.h>

#include <stdio.h>

#include "std/memory.h"
#include "std/debug.h"
#include "std/exception.h"
#include "std/resource.h"
#include "system/audio.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

#include "config.h"
#include "demo.h"

bool ExitDemo = false;

static void DoTimeSlice(TimeSliceT *slice, FrameT *frame, int thisFrame) {
  for (; slice->data.func; slice++) {
    bool invoke = false;

    if (slice->type < 0) {
      /* Recurse? */
      if ((slice->start <= thisFrame) && (thisFrame < slice->end))
        DoTimeSlice(slice->data.slice, frame, thisFrame);
    } else {
      int step = slice->type;

      switch (step) {
        case 0:
          /* Do it only once. */
          invoke = (slice->start <= thisFrame) && (slice->last < 0);
          if (invoke)
            slice->last = thisFrame;
          break;

        case 1:
          /* Do it every frame. */
          invoke = (slice->start <= thisFrame) && (thisFrame < slice->end);
          break;

        default:
          /* Do it every n-th frame. */
          if ((slice->start <= thisFrame) && (thisFrame < slice->end)) {
            if (slice->last == -1) {
              slice->last = slice->start;
              invoke = true;
            } else {
              if (thisFrame - slice->last >= step) {
                slice->last = thisFrame - ((thisFrame - slice->start) % step);
                invoke = true;
              }
            }
          }
          break;
      }

      if (invoke) {
        frame->first = slice->start;
        frame->last = slice->end - 1;
        frame->number = thisFrame - slice->start;

        slice->data.func(frame);
      }
    }
  }
}

/*
 * Makes all timing values absolute to the beginning of demo.
 */
static void CompileTimeSlice(TimeSliceT *slice, int firstFrame, int lastFrame) {
  for (; slice->name; slice++) {
    if (slice->start < 0)
      slice->start = lastFrame;
    else
      slice->start += firstFrame;

    if (slice->end < 0)
      slice->end = lastFrame;
    else
      slice->end += firstFrame;

    if (slice->type < 0)
      CompileTimeSlice(slice->data.slice, slice->start, slice->end);
    else {
      CallbackT *callback;

      for (callback = Callbacks; callback->name; callback++) {
        if (!strcmp(callback->name, slice->name)) {
          slice->data.func = callback->func;
          break;
        }
      }

      if (!slice->data.func)
        PANIC("Callback '%s' not found!", slice->name);
    }
  }
}

static void PrintTimeSlice(TimeSliceT *slice) {
  for (; slice->name != NULL; slice++) {
    char type[20];

    if (slice->type == -1)
      strcpy(type, "timeslice");
    else if (slice->type == 0)
      strcpy(type, "once");
    else if (slice->type == 1)
      strcpy(type, "each frame");
    else
      snprintf(type, sizeof(type), "every %d frames", slice->type);

    if (slice->type == -1) {
      printf("[%5d]: (start) %s\n", slice->start, slice->name);
      PrintTimeSlice(slice->data.slice);
      printf("[%5d]: (end) %s\n", slice->end, slice->name);
    } else if (slice->type == 0) {
      printf("[%5d - %5d]: (%s) {%p} %s\n",
             slice->start, slice->end, type, slice->data.func, slice->name);
    } else {
      printf("[%5d - %5d]: (%s) {%p} %s\n",
             slice->start, slice->end, type, slice->data.func, slice->name);
    }
  }
}

int main() {
  if (SystemCheck() && InitAudio() && ReadConfig()) {
    static bool ready = true;
    TimeSliceT *demo;
    float beat;

    StartResourceManager();
    StartEventQueue();

    demo = LoadTimeline();
    CompileTimeSlice(demo, 0, 5407);
    beat = GetBeatLength();

    TRY {
      if (!LoadDemo())
        PANIC("Loading demo failed.");
      LoadResources();
      SetupResources();
    }
    CATCH {
      ready = false;
    }

    if (ready) {
      BeginDemo();

      InstallVBlankIntServer();
      SetVBlankCounter(0);

      do {
        int frameNumber = GetVBlankCounter();
        FrameT frame = { .beat = beat };

        DoTimeSlice(demo, &frame, frameNumber);
        DisplaySwap();
        HandleEvents(frameNumber);
      } while (!ExitDemo);

      RemoveVBlankIntServer();

      KillDemo();
    }

    KillAudio();
    KillDisplay();
    StopEventQueue();
    StopResourceManager();

    MemUnref(DemoConfig);
  }

  return 0;
}
