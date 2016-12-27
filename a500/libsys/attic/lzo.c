#include "lzo.h"

/*
 * This routine was shamelessy stolen from minilzo 2.08 distribution.
 *
 * Changes from the original sources:
 * - run C preprocessor with m68k specific configuration,
 * - replaced types with their AmigaOS counterparts,
 * - transformed some loops to make gcc recognize and optimize them.
 *
 * If you think you can make the routine significantly faster by rewriting in
 * assembly then go for it! Remember to share the sources with Amiga community!
 */

LONG lzo1x_decompress(CONST UBYTE *in asm("a0"), ULONG in_len asm("d0"),
                      UBYTE *out asm("a1"), ULONG *out_len asm("a2"))
{
  UBYTE *op;
  CONST UBYTE *ip;
  UWORD t;
  CONST UBYTE *m_pos;
  CONST UBYTE *CONST ip_end = in + in_len;

  *out_len = 0;

  op = out;
  ip = in;

  if (*ip > 17) 
  {
    t = *ip++ - 17;
    if (t < 4)
      goto match_next;
    {
      t--;
      do *op++ = *ip++; while (--t != 0xffff);
    }
    goto first_literal_run;
  }

  for (;;) 
  {
    t = *ip++;
    if (t >= 16)
      goto match;
    if (t == 0) 
    {
      while (*ip == 0) 
      {
        t += 255;
        ip++;
      }
      t += 15 + *ip++;
    }

    {
      t += 2;
      do *op++ = *ip++; while (--t != 0xffff);
    }

first_literal_run:
    t = *ip++;
    if (t >= 16)
      goto match;

    m_pos = op - (1 + 0x0800);
    m_pos -= t >> 2;
    m_pos -= *ip++ << 2;

    *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos;

    goto match_done;

    for (;;) 
    {
match:
      if (t >= 64) 
      {
        m_pos = op - 1;
        m_pos -= (t >> 2) & 7;
        m_pos -= *ip++ << 3;
        t = (t >> 5) - 1;
        goto copy_match;
      }
      else if (t >= 32) 
      {
        t &= 31;
        if (t == 0)
        {
          while (*ip == 0)
          {
            t += 255;
            ip++;
          }
          t += 31 + *ip++;
        }

        m_pos = op - 1;
        m_pos -= *ip++ >> 2;
        m_pos -= *ip++ << 6;
      }
      else if (t >= 16)
      {
        m_pos = op;
        m_pos -= (t & 8) << 11;

        t &= 7;
        if (t == 0)
        {
          while (*ip == 0)
          {
            t += 255;
            ip++;
          }
          t += 7 + *ip++;
        }

        m_pos -= *ip++ >> 2;
        m_pos -= *ip++ << 6;

        if (m_pos == op)
          goto eof_found;

        m_pos -= 0x4000;
      }
      else
      {
        m_pos = op - 1;
        m_pos -= t >> 2;
        m_pos -= *ip++ << 2;

        *op++ = *m_pos++; *op++ = *m_pos;

        goto match_done;
      }

copy_match:
      {
        t++;
        do *op++ = *m_pos++; while (--t != 0xffff);
      }

match_done:
      t = ip[-2] & 3;
      if (t == 0)
        break;

match_next:
      {
        t--;
        do *op++ = *ip++; while (--t != 0xffff);
      }

      t = *ip++;
    }
  }

eof_found:
  *out_len = (ULONG)(op - out);

  if (ip == ip_end)
    return 0;
  if (ip < ip_end)
    return -8;
  return -4;
}
