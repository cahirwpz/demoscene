#include "interrupt.h"
#include "cia.h"
#include "keyboard.h"
#include "event.h"

static const char KeyMapLower[128] = {
  [KEY_BACKQUOTE] '`',
  [KEY_1] '1',
  [KEY_2] '2',
  [KEY_3] '3',
  [KEY_4] '4',
  [KEY_5] '5',
  [KEY_6] '6',
  [KEY_7] '7',
  [KEY_8] '8',
  [KEY_9] '9',
  [KEY_0] '0',
  [KEY_MINUS] '-',
  [KEY_EQUAL] '=',
  [KEY_BACKSLASH] '\\',
  [KEY_KP_0] '0',
  [KEY_Q] 'q',
  [KEY_W] 'w',
  [KEY_E] 'e',
  [KEY_R] 'r',
  [KEY_T] 't',
  [KEY_Y] 'y',
  [KEY_U] 'u',
  [KEY_I] 'i',
  [KEY_O] 'o',
  [KEY_P] 'p',
  [KEY_LBRACKET] '[',
  [KEY_RBRACKET] ']',
  [KEY_KP_1] '1',
  [KEY_KP_2] '2',
  [KEY_KP_3] '3',
  [KEY_A] 'a',
  [KEY_S] 's',
  [KEY_D] 'd',
  [KEY_F] 'f',
  [KEY_G] 'g',
  [KEY_H] 'h',
  [KEY_J] 'j',
  [KEY_K] 'k',
  [KEY_L] 'l',
  [KEY_SEMICOLON] ';',
  [KEY_QUOTE] '\'',
  [KEY_KP_4] '4',
  [KEY_KP_5] '5',
  [KEY_KP_6] '6',
  [KEY_Z] 'z',
  [KEY_X] 'x',
  [KEY_C] 'c',
  [KEY_V] 'v',
  [KEY_B] 'b',
  [KEY_N] 'n',
  [KEY_M] 'm',
  [KEY_COMMA] ',',
  [KEY_PERIOD] '.',
  [KEY_SLASH] '/',
  [KEY_KP_PERIOD] '.',
  [KEY_KP_7] '7',
  [KEY_KP_8] '8',
  [KEY_KP_9] '9',
  [KEY_SPACE] ' ',
  [KEY_BACKSPACE] '\b',
  [KEY_TAB] '\t',
  [KEY_KP_ENTER] '\n',
  [KEY_RETURN] '\n',
  [KEY_ESCAPE] '\033',
  [KEY_KP_MINUS] '-',
  [KEY_KP_LPAREN] '(',
  [KEY_KP_RPAREN] ')',
  [KEY_KP_DIVIDE] '/',
  [KEY_KP_MULTIPLY] '*',
  [KEY_KP_PLUS] '+'
};

static const char KeyMapUpper[128] = {
  [KEY_BACKQUOTE] '~',
  [KEY_1] '!',
  [KEY_2] '@',
  [KEY_3] '#',
  [KEY_4] '$',
  [KEY_5] '%',
  [KEY_6] '^',
  [KEY_7] '&',
  [KEY_8] '*',
  [KEY_9] '(',
  [KEY_0] ')',
  [KEY_MINUS] '_',
  [KEY_EQUAL] '+',
  [KEY_BACKSLASH] '|',
  [KEY_Q] 'Q',
  [KEY_W] 'W',
  [KEY_E] 'E',
  [KEY_R] 'R',
  [KEY_T] 'T',
  [KEY_Y] 'Y',
  [KEY_U] 'U',
  [KEY_I] 'I',
  [KEY_O] 'O',
  [KEY_P] 'P',
  [KEY_LBRACKET] '{',
  [KEY_RBRACKET] '}',
  [KEY_A] 'A',
  [KEY_S] 'S',
  [KEY_D] 'D',
  [KEY_F] 'F',
  [KEY_G] 'G',
  [KEY_H] 'H',
  [KEY_J] 'J',
  [KEY_K] 'K',
  [KEY_L] 'L',
  [KEY_SEMICOLON] ':',
  [KEY_QUOTE] '"',
  [KEY_Z] 'Z',
  [KEY_X] 'X',
  [KEY_C] 'C',
  [KEY_V] 'V',
  [KEY_B] 'B',
  [KEY_N] 'N',
  [KEY_M] 'M',
  [KEY_COMMA] '<',
  [KEY_PERIOD] '>',
  [KEY_SLASH] '?'
};

static u_char modifier;

static void PushKeyEvent(u_char raw) {
  KeyEventT event;
  u_char code = raw & 0x7f;
  u_char mod;

  if (code == KEY_LSHIFT)
    mod = MOD_LSHIFT;
  else if (code == KEY_RSHIFT)
    mod = MOD_RSHIFT;
  else if (code == KEY_CONTROL)
    mod = MOD_CONTROL;
  else if (code == KEY_LALT)
    mod = MOD_LALT;
  else if (code == KEY_RALT)
    mod = MOD_RALT;
  else if (code == KEY_LAMIGA)
    mod = MOD_LAMIGA;
  else if (code == KEY_RAMIGA)
    mod = MOD_RAMIGA;
  else
    mod = 0;

  /* Process key modifiers */
  if (raw & 0x80)
    modifier &= ~(mod | MOD_PRESSED);
  else
    modifier |= (mod | MOD_PRESSED);

  event.type = EV_KEY;
  event.code = code;
  event.modifier = modifier;
  event.ascii = ((modifier & MOD_SHIFT) ? KeyMapUpper : KeyMapLower)[code];

  PushEvent((EventT *)&event);
}

static __interrupt int KeyboardIntHandler(void) {
  if (SampleICR(ciaa, CIAICRF_SP)) {
    /* Read keyboard data register. */
    u_char sdr = ciaa->ciasdr;
    /* Set serial port to output mode. */
    ciaa->ciacra |= CIACRAF_SPMODE;
    /* Send handshake. */
    ciaa->ciasdr = 0;
    /* Save raw key in the queue. Filter out exceptional conditions. */
    {
      u_char raw = ((sdr >> 1) | (sdr << 7)) ^ 0xff;
      if (raw != 0x78 && /* Reset warning. */
          raw != 0xf9 && /* Last key code bad. */
          raw != 0xfa && /* Keyboard key buffer overflow. */
          raw != 0xfc && /* Keyboard self-test fail. */
          raw != 0xfd && /* Initiate power-up key stream. */
          raw != 0xfe)   /* Terminate power-up key stream. */
        PushKeyEvent(raw);
    }
    /* Wait for at least 85us for handshake to be registered. */
    WaitTimerB(ciab, TIMER_US(85));
    /* Set back to input mode. */
    ciaa->ciacra &= (u_char)~CIACRAF_SPMODE;
  }

  return 0;
}

INTERRUPT(KeyboardInterrupt, -5, KeyboardIntHandler, NULL);

void KeyboardInit() {
  /* Enable keyboard interrupt.
   * The keyboard is attached to CIA-A serial port. */
  WriteICR(ciaa, CIAICRF_SETCLR | CIAICRF_SP);
  ciaa->ciacra = (u_char)(~CIACRAF_SPMODE);

  AddIntServer(INTB_PORTS, KeyboardInterrupt);
}

void KeyboardKill() {
  RemIntServer(INTB_PORTS, KeyboardInterrupt);
}
