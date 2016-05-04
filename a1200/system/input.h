#ifndef __SYSTEM_EVENT_QUEUE_H__
#define __SYSTEM_EVENT_QUEUE_H__

#include <devices/inputevent.h>

#include "std/types.h"

#define KEY_BACKQUOTE   0x00
#define KEY_1           0x01
#define KEY_2           0x02
#define KEY_3           0x03
#define KEY_4           0x04
#define KEY_5           0x05
#define KEY_6           0x06
#define KEY_7           0x07
#define KEY_8           0x08
#define KEY_9           0x09
#define KEY_0           0x0A
#define KEY_MINUS       0x0B
#define KEY_EQUALS      0x0C
#define KEY_BACKSLASH   0x0D
#define KEY_KP0         0x0F

#define KEY_Q           0x10
#define KEY_W           0x11
#define KEY_E           0x12
#define KEY_R           0x13
#define KEY_T           0x14
#define KEY_Y           0x15
#define KEY_U           0x16
#define KEY_I           0x17
#define KEY_O           0x18
#define KEY_P           0x19
#define KEY_LBRACKET    0x1A
#define KEY_RBRACKET    0x1B
#define KEY_KP1         0x1D
#define KEY_KP2         0x1E
#define KEY_KP3         0x1F

#define KEY_A           0x20
#define KEY_S           0x21
#define KEY_D           0x22
#define KEY_F           0x23
#define KEY_G           0x24
#define KEY_H           0x25
#define KEY_J           0x26
#define KEY_K           0x27
#define KEY_L           0x28
#define KEY_COLON       0x29
#define KEY_QUOTE       0x2A
#define KEY_DEADKEY1    0x2B
#define KEY_KP4         0x2D
#define KEY_KP5         0x2E
#define KEY_KP6         0x2F

#define KEY_DEADKEY2    0x30
#define KEY_Z           0x31
#define KEY_X           0x32
#define KEY_C           0x33
#define KEY_V           0x34
#define KEY_B           0x35
#define KEY_N           0x36
#define KEY_M           0x37
#define KEY_PERIOD      0x38
#define KEY_COMMA       0x39
#define KEY_SLASH       0x3A
#define KEY_KP_PERIOD   0x3C
#define KEY_KP7         0x3D
#define KEY_KP8         0x3E
#define KEY_KP9         0x3F

#define KEY_SPACE       0x40
#define KEY_BACKSPACE   0x41
#define KEY_TAB         0x42
#define KEY_ENTER       0x43
#define KEY_RETURN      0x44
#define KEY_ESCAPE      0x45
#define KEY_DELETE      0x46
#define KEY_KP_MINUS    0x4A
#define KEY_UP          0x4C
#define KEY_DOWN        0x4D
#define KEY_RIGHT       0x4E
#define KEY_LEFT        0x4F

#define KEY_F1          0x50
#define KEY_F2          0x51
#define KEY_F3          0x52
#define KEY_F4          0x53
#define KEY_F5          0x54
#define KEY_F6          0x55
#define KEY_F7          0x56
#define KEY_F8          0x57
#define KEY_F9          0x58
#define KEY_F10         0x59
#define KEY_KP_LPAREN   0x5A
#define KEY_KP_RPAREN   0x5B
#define KEY_KP_DIVIDE   0x5C
#define KEY_KP_MULTIPLY 0x5D
#define KEY_KP_PLUS     0x5E
#define KEY_HELP        0x5F

#define KEY_LSHIFT      0x60
#define KEY_RSHIFT      0x61
#define KEY_CAPSLOCK    0x62
#define KEY_CONTROL     0x63
#define KEY_LALT        0x64
#define KEY_RALT        0x65
#define KEY_LAMIGA      0x66
#define KEY_RAMIGA      0x67

typedef struct InputEvent InputEventT;

void StartEventQueue();
void StopEventQueue();

void EventQueueReset();
bool EventQueuePop(InputEventT *event);

#define KEY_PRESSED(ev, key) \
  (((ev)->ie_Class == IECLASS_RAWKEY) && \
   (((ev)->ie_Code & IECODE_UP_PREFIX) == (key)))

#define KEY_RELEASED(ev, key) \
  (((ev)->ie_Class == IECLASS_RAWKEY) && \
   ((ev)->ie_Code & IECODE_UP_PREFIX) && \
   (((ev)->ie_Code & ~IECODE_UP_PREFIX) == (key)))

#endif
