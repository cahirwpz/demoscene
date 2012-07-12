#include "gfx/colors.h"
#include "gfx/pixbuf.h"
#include "txtgen/txtgen.h"

struct Channel {
  PixBufT *pixbuf;
  ComponentT component;

  /* private */
  size_t offset;
};

void ChannelSetActiveComponent(ChannelT *channel, ComponentT component) {
  channel->component = component;

  switch (component) {
    case COMPONENT_R:
      channel->offset = offsetof(ColorT, r);
      break;
    case COMPONENT_G:
      channel->offset = offsetof(ColorT, g);
      break;
    case COMPONENT_B:
      channel->offset = offsetof(ColorT, b);
      break;
  }
}

size_t GetSample(ChannelT *channel asm("a0"), size_t index asm("d0")) {
  uint32_t *data = (uint32_t *)&channel->pixbuf->data[channel->offset];
  return *((uint8_t *)&data[index]);
}

void SetSample(ChannelT *channel asm("a0"), size_t index asm("d0"), size_t value asm("d1")) {
  uint32_t *data = (uint32_t *)&channel->pixbuf->data[channel->offset];
  *((uint8_t *)&data[index]) = value;
}

size_t GetChannelSize(ChannelT *channel asm("a0")) {
  return channel->pixbuf->width * channel->pixbuf->height;
}
