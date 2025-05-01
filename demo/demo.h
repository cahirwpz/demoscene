#ifndef __DEMO_H__
#define __DEMO_H__

#include <bitmap.h>
#include <palette.h>
#include <sync.h>
#include <effect.h>

short UpdateFrameCount(void);

static inline short FromCurrKeyFrame(TrackT *track) {
  return frameCount - CurrKeyFrame(track);
}

static inline short TillNextKeyFrame(TrackT *track) {
  return NextKeyFrame(track) - frameCount;
}

#endif /* !__DEMO_H__ */
