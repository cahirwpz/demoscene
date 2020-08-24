#include "interrupt.h"
#include "cia.h"
#include "keyboard.h"
#include "event.h"
#include "debug.h"

#define LO(K, V) [K] = V
#define HI(K, V) [K | 0x80] = V

/* clang-format off */
static const u_char KeyMap[256] = {
  LO(KEY_BACKQUOTE, '`'),
  LO(KEY_1, '1'),
  LO(KEY_2, '2'),
  LO(KEY_3, '3'),
  LO(KEY_4, '4'),
  LO(KEY_5, '5'),
  LO(KEY_6, '6'),
  LO(KEY_7, '7'),
  LO(KEY_8, '8'),
  LO(KEY_9, '9'),
  LO(KEY_0, '0'),
  LO(KEY_MINUS, '-'),
  LO(KEY_EQUAL, '='),
  LO(KEY_BACKSLASH, '\\'),
  LO(KEY_KP_0, '0'),
  LO(KEY_Q, 'q'),
  LO(KEY_W, 'w'),
  LO(KEY_E, 'e'),
  LO(KEY_R, 'r'),
  LO(KEY_T, 't'),
  LO(KEY_Y, 'y'),
  LO(KEY_U, 'u'),
  LO(KEY_I, 'i'),
  LO(KEY_O, 'o'),
  LO(KEY_P, 'p'),
  LO(KEY_LBRACKET, '['),
  LO(KEY_RBRACKET, ']'),
  LO(KEY_KP_1, '1'),
  LO(KEY_KP_2, '2'),
  LO(KEY_KP_3, '3'),
  LO(KEY_A, 'a'),
  LO(KEY_S, 's'),
  LO(KEY_D, 'd'),
  LO(KEY_F, 'f'),
  LO(KEY_G, 'g'),
  LO(KEY_H, 'h'),
  LO(KEY_J, 'j'),
  LO(KEY_K, 'k'),
  LO(KEY_L, 'l'),
  LO(KEY_SEMICOLON, ';'),
  LO(KEY_QUOTE, '\''),
  LO(KEY_KP_4, '4'),
  LO(KEY_KP_5, '5'),
  LO(KEY_KP_6, '6'),
  LO(KEY_Z, 'z'),
  LO(KEY_X, 'x'),
  LO(KEY_C, 'c'),
  LO(KEY_V, 'v'),
  LO(KEY_B, 'b'),
  LO(KEY_N, 'n'),
  LO(KEY_M, 'm'),
  LO(KEY_COMMA, ','),
  LO(KEY_PERIOD, '.'),
  LO(KEY_SLASH, '/'),
  LO(KEY_KP_PERIOD, '.'),
  LO(KEY_KP_7, '7'),
  LO(KEY_KP_8, '8'),
  LO(KEY_KP_9, '9'),
  LO(KEY_SPACE, ' '),
  LO(KEY_BACKSPACE, '\b'),
  LO(KEY_TAB, '\t'),
  LO(KEY_KP_ENTER, '\n'),
  LO(KEY_RETURN, '\n'),
  LO(KEY_ESCAPE, '\033'),
  LO(KEY_KP_MINUS, '-'),
  LO(KEY_KP_LPAREN, '('),
  LO(KEY_KP_RPAREN, ')'),
  LO(KEY_KP_DIVIDE, '/'),
  LO(KEY_KP_MULTIPLY, '*'),
  LO(KEY_KP_PLUS, '+'),
  LO(KEY_LSHIFT, MOD_LSHIFT),
  LO(KEY_RSHIFT, MOD_RSHIFT),
  LO(KEY_CONTROL, MOD_CONTROL),
  LO(KEY_LALT, MOD_LALT),
  LO(KEY_RALT, MOD_RALT),
  LO(KEY_LAMIGA, MOD_LAMIGA),
  LO(KEY_RAMIGA, MOD_RAMIGA),
  HI(KEY_BACKQUOTE, '~'),
  HI(KEY_1, '!'),
  HI(KEY_2, '@'),
  HI(KEY_3, '#'),
  HI(KEY_4, '$'),
  HI(KEY_5, '%'),
  HI(KEY_6, '^'),
  HI(KEY_7, '&'),
  HI(KEY_8, '*'),
  HI(KEY_9, '('),
  HI(KEY_0, ')'),
  HI(KEY_MINUS, '_'),
  HI(KEY_EQUAL, '+'),
  HI(KEY_BACKSLASH, '|'),
  HI(KEY_Q, 'Q'),
  HI(KEY_W, 'W'),
  HI(KEY_E, 'E'),
  HI(KEY_R, 'R'),
  HI(KEY_T, 'T'),
  HI(KEY_Y, 'Y'),
  HI(KEY_U, 'U'),
  HI(KEY_I, 'I'),
  HI(KEY_O, 'O'),
  HI(KEY_P, 'P'),
  HI(KEY_LBRACKET, '{'),
  HI(KEY_RBRACKET, '}'),
  HI(KEY_A, 'A'),
  HI(KEY_S, 'S'),
  HI(KEY_D, 'D'),
  HI(KEY_F, 'F'),
  HI(KEY_G, 'G'),
  HI(KEY_H, 'H'),
  HI(KEY_J, 'J'),
  HI(KEY_K, 'K'),
  HI(KEY_L, 'L'),
  HI(KEY_SEMICOLON, ':'),
  HI(KEY_QUOTE, '"'),
  HI(KEY_Z, 'Z'),
  HI(KEY_X, 'X'),
  HI(KEY_C, 'C'),
  HI(KEY_V, 'V'),
  HI(KEY_B, 'B'),
  HI(KEY_N, 'N'),
  HI(KEY_M, 'M'),
  HI(KEY_COMMA, '<'),
  HI(KEY_PERIOD, '>'),
  HI(KEY_SLASH, '?'),

  /* Error codes have value set to -1 */
  [0x78] = -1, /* Reset warning. */
  [0xf9] = -1, /* Last key code bad. */
  [0xfa] = -1, /* Keyboard key buffer overflow. */
  [0xfc] = -1, /* Keyboard self-test fail. */
  [0xfd] = -1, /* Initiate power-up key stream. */
  [0xfe] = -1, /* Terminate power-up key stream. */
  [0xff] = -1  /* ??? */
};
/* clang-format on */

static u_char modifier;

static void PushKeyEvent(u_char raw) {
  KeyEventT ev;
  u_char code = raw & 0x7f;
  u_char change = code >= KEY_LSHIFT ? KeyMap[code] : 0;

  /* Process key modifiers */
  if (raw & 0x80)
    modifier &= ~(change | MOD_PRESSED);
  else
    modifier |= (change | MOD_PRESSED);

  ev.type = EV_KEY;
  ev.code = code;
  ev.modifier = modifier;
  if (code < KEY_LSHIFT)
    ev.ascii = KeyMap[code + ((modifier & MOD_SHIFT) ? 128 : 0)];
  else
    ev.ascii = 0;

  PushEventISR((EventT *)&ev);
}

static int KeyboardIntHandler(void) {
  if (SampleICR(ciaa, CIAICRF_SP)) {
    /* Read keyboard data register. Yeah, it's negated. */
    uint8_t sdr = ~ciaa->ciasdr;
    /* Send handshake.
     * 1) Set serial port to output mode.
     * 2) Wait for at least 85us for handshake to be registered.
     * 3) Set back to input mode. */
    ciaa->ciacra |= CIACRAF_SPMODE;
    WaitTimerB(ciab, TIMER_US(85));
    ciaa->ciacra &= ~CIACRAF_SPMODE;
    /* Save raw key in the queue. Filter out exceptional conditions. */
    {
      u_char raw = (sdr >> 1) | (sdr << 7);
      if (KeyMap[raw] != (u_char)-1)
        PushKeyEvent(raw);
    }
  }

  return 0;
}

INTSERVER(KeyboardServer, -5, (IntFuncT)KeyboardIntHandler, NULL);

void KeyboardInit(void) {
  Log("[Init] Keyboard driver!\n");
  /* Set to input mode. */
  ciaa->ciacra &= ~CIACRAF_SPMODE;
  /* Enable keyboard interrupt.
   * The keyboard is attached to CIA-A serial port. */
  WriteICR(ciaa, CIAICRF_SETCLR | CIAICRF_SP);

  AddIntServer(PortsChain, KeyboardServer);
}

void KeyboardKill(void) {
  RemIntServer(PortsChain, KeyboardServer);
}
