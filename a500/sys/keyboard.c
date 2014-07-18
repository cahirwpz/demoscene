#include "hardware.h"
#include "keyboard.h"

static char KeyMapLower[128] = {
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

static char KeyMapUpper[128] = {
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
  [KEY_W] 'V',
  [KEY_B] 'B',
  [KEY_N] 'N',
  [KEY_M] 'M',
  [KEY_COMMA] '<',
  [KEY_PERIOD] '>',
  [KEY_SLASH] '?'
};

KeyModT KeyMod = 0;
KeyCodeT KeyCode = 0;
char KeyChar = 0;

void KeyboardIntHandler() {
  if (ciaa->ciaicr & CIAICRF_SP) {
    UBYTE sdr = ciaa->ciasdr;
    UBYTE raw = ((sdr >> 1) | (sdr << 7)) ^ 0xff;

    /* Set serial port to output mode. */
    ciaa->ciacra |= CIACRAF_SPMODE;
    /* Send handshake. */
    ciaa->ciasdr = 0;
    /* Set back to input mode. */
    ciaa->ciacra ^= CIACRAF_SPMODE;

    {
      UBYTE code = raw & 0x7f;
      UBYTE mod = 0;

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

      /* Process key modifiers */
      if (raw & 0x80)
        KeyMod &= ~(mod | MOD_PRESSED);
      else
        KeyMod |= (mod | MOD_PRESSED);

      KeyCode = code;
      KeyChar = ((KeyMod & MOD_SHIFT) ? KeyMapUpper : KeyMapLower)[code];
    }
  }
}
