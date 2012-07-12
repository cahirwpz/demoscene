#ifndef __TXTGEN_H__
#define __TXTGEN_H__

typedef enum {COMPONENT_R, COMPONENT_G, COMPONENT_B} ComponentT;
typedef struct Channel ChannelT;

void ChannelSetActiveComponent(ChannelT *channel, ComponentT component);

void ChannelClear(ChannelT *D, uint8_t value);
void ChannelAdd(ChannelT *D, ChannelT *A, ChannelT *B);
void ChannelMul(ChannelT *D, ChannelT *A, ChannelT *B);
void ChannelMix(ChannelT *D, ChannelT *A, ChannelT *B, size_t percent);
void ChannelCopy(ChannelT *D, ChannelT *A);
void ChannelSwap(ChannelT *D, ChannelT *A);
void ChannelMax(ChannelT *D, ChannelT *A);
void ChannelShade(ChannelT *D, ChannelT *A, ChannelT *B);
void ChannelMixWithMap(ChannelT *D, ChannelT *A, ChannelT *B, ChannelT *C);

#endif
