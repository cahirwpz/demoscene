// This file contains an emulator of very basic features of
// Amiga Original Chipset display, with some extra tools.
final int ChipMemSize = 1024 * 1024;
final int OcsRasterWidth = 752;
final int OcsRasterHeight = 572;
final float OcsFrameRate = 50.0;

// Represents a chunk of chip memory addressable from 0 to size-1
class Memory {
  short data[];
  int address;
  int size;
  
  Memory(short _data[], int _address, int _size) {
    data = _data;
    address = _address;
    size = _size;
  }
  
  Memory(int _size) {
    data = new short[_size];
    address = 0;
    size = _size;
  }
      
  short read(int i) {
    assert i < size;
    return data[address + i];
  }
  
  void write(int i, int v) {
    assert i < size;
    data[address + i] = (short)v;
  }
};

class ChipMemory extends Memory {
  int lastFree;
  
  private int nwords(int nbytes) {
    return (nbytes + 1) / 2;
  }

  ChipMemory() {
    super(ChipMemSize / 2);
    lastFree = 0;
  }
  
  int alloc(int n) {
    assert lastFree + nwords(n) < ChipMemSize / 2;
    int address = lastFree;
    lastFree += nwords(n);
    return address;
  }
  
  Bitplane allocBitplane(int _width, int _height) {
    assert _width % 16 == 0; // must be multiple of 16
    int size = _width * _height / 8;
    int address = alloc(size);
    return new Bitplane(data, address, size, _width, _height);
  }

  Bitplane[] allocBitmap(int _width, int _height, int _depth) {
    Bitplane bpl[] = new Bitplane[_depth];
    for (int i = 0; i < _depth; i++)
      bpl[i] = allocBitplane(_width, _height);
    return bpl;
  }

  Sprite allocSprite(int _height, int _chunks) {
    int size = (_height + _chunks + 1) * 2;
    int address = alloc(size);
    return new Sprite(data, address, size);
  }
};

class Bitplane extends Memory {
  // Drawing routines make use of size embedded into Bitplane class
  int width;
  int height;
  
  Bitplane(short memory[], int start, int size, int _width, int _height) {
    super(memory, start, size);
    width = _width;
    height = _height;
  }

  private int bit(int x) {
    return 1 << (x & 15);
  }

  private int pos(int x, int y) {
    return ((y * width) >> 4) + (x >> 4);
  }

  int size() {
    return width * height / 16;
  }

  boolean get(int x, int y) {
    int i = pos(x, y);
    return boolean(read(i) & bit(x));
  }

  void set(int x, int y, boolean value) {
    if (value) bset(x, y);
    else bclr(x, y);
  }

  void bset(int x, int y) {
    int i = pos(x, y);
    write(i, read(i) | bit(x));
  }

  void bclr(int x, int y) {
    int i = pos(x, y);
    write(i, read(i) & ~bit(x));
  }

  void bxor(int x, int y) {
    if (x < 0 || x >= width) return;
    if (y < 0 || y >= height) return;
    int i = pos(x, y);
    write(i, read(i) ^ bit(x));
  }
};

// Limited representation of copper instruction.
class CopIns {
  int n;
  color c;

  CopIns(int _n, color _c) {
    n = _n;
    c = _c;
  }
};


static class SpriteData {
  /*
   * SPRxPOS:
   *  Bits 15-8 contain the low 8 bits of VSTART
   *  Bits 7-0 contain the high 8 bits of HSTART
   */
  static int Pos(int hstart, int vstart) {
    return ((vstart & 255) << 8) | ((hstart >> 1) & 255);
  }

  /*
   * SPRxCTL:
   *  Bits 15-8       The low eight bits of VSTOP
   *  Bit 7           (Used in attachment)
   *  Bits 6-3        Unused (make zero)
   *  Bit 2           The VSTART high bit
   *  Bit 1           The VSTOP high bit
   *  Bit 0           The HSTART low bit
   */
  static int Ctl(int hstart, int vstart, int h, boolean attached) {
    int vstop = vstart + h + 1;
    return ((vstop & 255) << 8) |
           (attached ? 128 : 0) |
           ((vstart & 256) >> 6) |
           ((vstop & 256) >> 7) |
           (hstart & 1);
  }

  static int VStart(int pos, int ctl) {
    return ((pos >> 8) & 255) | ((ctl & 4) << 6);
  }

  static int HStart(int pos, int ctl) {
    return ((pos & 255) << 1) | (ctl & 1);
  }
  
  static int VStop(int ctl) {
    return ((ctl >> 8) & 255) | ((ctl & 2) << 7);
  }
  
  static boolean Attached(int ctl) {
    return boolean(ctl & 128);
  }
} 

// Sprite is always 16 pixels wide.
class Sprite extends Memory {  
  Sprite(int _height, int _chunks) {
    super((_height + _chunks + 1) * 2);
  }
  
  Sprite(short memory[], int start, int size) {
    super(memory, start, size);
  }
  
  void header(int i, int x, int y, int h, boolean attached) {
    write(i * 2, SpriteData.Pos(x, y));
    write(i * 2 + 1, SpriteData.Ctl(x, y, h, attached));
  }

  void end(int i) {
    header(i, 0, 0, 0, false);
  }
  
  void update(int i, int x, int y) {
    int pos = read(i * 2);
    int ctl = read(i * 2 + 1);

    int vstart = SpriteData.VStart(pos, ctl);
    int vstop = SpriteData.VStop(ctl);
    boolean attached = SpriteData.Attached(ctl);
    
    header(i, x, y, vstop - vstart - 1, attached);
  }
  
  int get(int i, int x) {
    int p = (read(i * 2 + 0) >> (15 - x)) & 1;
    int q = (read(i * 2 + 1) >> (15 - x)) & 1;
    
    return (p << 1) | q;
  }

  void set(int i, int x, int v) {
    int p = (v >> 1) & 1;
    int q = v & 1;
    
    write(i * 2 + 0, read(i * 2 + 0) | p << (15 - x));
    write(i * 2 + 1, read(i * 2 + 1) | q << (15 - x));
  }
};

// Limits color space to OCS 12-bit RGB.
color rgb12(color c) {
  int r = (c & 0xf00000) >> 20;
  int g = (c & 0x00f000) >> 12;
  int b = (c & 0x0000f0) >> 4;
  return color((r << 4) | r, (g << 4) | g, (b << 4) | b);
}

int HPOS(int x) {
  return 0x81 + x;
}

int VPOS(int y) {
  return 0x2c + y;
}

// Some basic properties of OCS display
final int MaxColorClk = 227;
final int MaxVertPos = 312;
final int HBlankEnd = 78;
final int VBlankEnd = 26;
final int MaxSprites = 8;
final int FirstSpriteCycle = 0x14;
final int LastSpriteCycle = FirstSpriteCycle + MaxSprites * 4;
final int ClockFreq = 3546895;
final int BplFetchSlot[] = {-1, 3, 5, 1, -1, 2, 4, 0};

// BPLCON0 register values
final int BPLCON0_DBLPF = 0x0400;
final int BPLCON0_HOMOD = 0x0800;
final int BPLCON0_BPU_MASK = 0x7000;
final int BPLCON0_BPU_SHIFT = 12;

// BPLCON1 register values
final int BPLCON1_PF2H_MASK = 0xF0;
final int BPLCON1_PF2H_SHIFT = 4;
final int BPLCON1_PF1H_MASK = 0x0F0;
final int BPLCON1_PF1H_SHIFT = 0;

// BPLCON2 register values
final int BPLCON2_PF2PRI = 0x40;
final int BPLCON2_PF2P_MASK = 0x38;
final int BPLCON2_PF2P_SHIFT = 3;
final int BPLCON2_PF1P_MASK = 0x07;
final int BPLCON2_PF1P_SHIFT = 0;

// DMACON register values
final int DMAF_AUD0 = 0x0001;
final int DMAF_AUD1 = 0x0002;
final int DMAF_AUD2 = 0x0004;
final int DMAF_AUD3 = 0x0008;
final int DMAF_DISK = 0x0010;
final int DMAF_SPRITE = 0x0020;
final int DMAF_COPPER = 0x0080;
final int DMAF_RASTER = 0x0100;

public enum SpriteStatus { POSCTL, DATA, END };

class OrigChipSet {
  ChipMemory chip;

  // DMA channels state
  int DMACON;
  
  // Display window state
  int DIWSTRT;
  int DIWSTOP;
  
  // Bitplanes fetch & display state
  int DDFSTRT;
  int DDFSTOP;
  int BPLPT[];
  int BPLDAT[];
  int BPLCON0;
  int BPLCON1;
  int BPLCON2;
  int BPL1MOD;
  int BPL2MOD;
    
  // Sprites fetch & display state
  int SPRPT[];
  int SPRCTL[];
  int SPRPOS[];
  int SPRDATA[];
  int SPRDATB[];
  
  // Copper state
  int COPINS[];
  int COP1LC;
  int COP2LC;
  int COPJMP1;
  int COPJMP2;
  
  // Original chipset allows to set up 32 colors. In EHB-mode,
  // Amiga can display 64 colors, where second half is at half
  // the brightness of first half.
  int COLOR[];

  // internal state
  int bpldat[];
  int bpldatReady;
  SpriteStatus sprstat[];
 
  OrigChipSet() {
    chip = new ChipMemory();
 
    BPLPT  = new int[6];
    BPLDAT = new int[6];
    SPRPT = new int[8];
    SPRDATA = new int[8];
    SPRDATB = new int[8];
    COPINS = new int[2];
    COLOR = new int[32];
    
    for (int i = 0; i < MaxSprites; i++)
      SPRPT[i] = -1;

    bpldat = new int[16];
    sprstat = new SpriteStatus[8];
  }
  
  int width() {
    return OcsRasterWidth / 2;
  }
  
  int height() {
    return OcsRasterHeight / 2;
  }

  int depth() {
    return (BPLCON0 & BPLCON0_BPU_MASK) >> BPLCON0_BPU_SHIFT;
  }
  
  int vstart() {
    return DIWSTRT >> 8;
  }
  
  int vstop() {
    return 256 + (DIWSTOP >> 8);
  }
  
  int hstart() {
    return DIWSTRT & 255;
  }
  
  int hstop() {
    return 256 + (DIWSTOP & 255);
  }
  
  void enableDMA(int flags) {
    DMACON |= flags;
  }

  void setColor(int i, color c) {
    assert i >= 0 && i < 32;
    COLOR[i] = rgb12(c);
  }

  /*
   * DDFSTRT and DDFSTOP have resolution of a color clock.
   *
   * Only bits 7..2 of DDFSTRT and DDFSTOP are meaningful on OCS!
   *
   * Values to determine Display Data Fetch Start and Stop must be divisible
   * by 16 pixels, because hardware fetches bitplanes in 16-bit word units.
   * Bitplane fetcher uses 8 clocks for LoRes (1 clock = 2 pixels) to fetch
   * enough data to display it.
   *
   * HS = Horizontal Start, W = Width (divisible by 16)
   *
   * For LoRes: DDFSTART = HS / 2 - 8.5, DDFSTOP = DDFSTRT + W / 2 - 8
   */
  void setupBitplaneFetch(int xs, int w) {
    xs -= 17;
    w >>= 1;

    DDFSTRT = (xs >> 1) & ~7; /* 8 clock resolution */
    DDFSTOP = DDFSTRT + w - 8;
    BPLCON1 = ((xs & 15) << 4) | (xs & 15);
  }

  /* Arguments must be always specified in low resolution coordinates. */
  void setupDisplayWindow(int xs, int ys, int w, int h) {
    /* vstart  $00 ..  $ff */
    /* hstart  $00 ..  $ff */
    /* vstop   $80 .. $17f */
    /* hstop  $100 .. $1ff */
    int xe = xs + w;
    int ye = ys + h;
  
    DIWSTRT = ((ys & 255) << 8) | (xs & 255);
    DIWSTOP = ((ye & 255) << 8) | (xe & 255);
  }

  void setupScreen(Bitplane[] bitmap, int _mode, int _depth) {
    assert _depth >= 1 && _depth <= 6;
    
    BPLCON0 = _mode | (_depth << BPLCON0_BPU_SHIFT);
    BPL1MOD = 0;
    BPL2MOD = 0;
    for (int i = 0; i < _depth; i++)
      BPLPT[i] = bitmap[i].address;
  }
 
  /*
  // Sprite DMA simulation.
  int sprmask = 0;
  
  for (int j = 0; j < 8; j++) {
    if (spr[j] == null)
      continue;
    if (y < spr[j].y || y >= spr[j].y + spr[j].height)
      continue;
      
    int sy = y - spr[j].y;
    
    _spr[j].attached = spr[j].attached;
    _spr[j].x = spr[j].x;
    _spr[j].write(0, spr[j].read(2 * sy));
    _spr[j].write(1, spr[j].read(2 * sy + 1));
    
    sprmask |= 1 << j;
  }

  for (int j = 7; j >= 0; j--) {
    if ((sprmask & (1 << j)) == 0)
      continue;
    if (x < _spr[j].x || x >= _spr[j].x + 16)
      continue;

    if (_spr[j].attached) {
      sprmask &= ~(1 << (j - 1));
      
      int w0 = _spr[j].get(x - spr[j].x, 0);
      int w1 = _spr[j - 1].get(x - spr[j - 1].x, 0);
      int w = (w0 << 2) | w1;

      if (w > 0)
        v = w + 16;
    } else {
      int w = _spr[j].get(x - spr[j].x, 0);
      
      if (w > 0)
        v = w + (j & ~1) * 2 + 16;
    }
  }
  */

  boolean fetchBitplane(int vpos, int cycle) {
    if ((DMACON & DMAF_RASTER) == 0)
      return false;
      
    if (vpos < VBlankEnd)
      return false;
    
    if (vstart() > vpos || vpos >= vstop())
      return false;
      
    if (cycle < DDFSTRT || cycle > DDFSTOP + 8)
      return false;
      
    int slot = (cycle - DDFSTRT) % 8;
    int bpl = BplFetchSlot[slot];
    if (bpl < 0 || depth() <= bpl)
      return false;
    
    // Fetch bitplane data
    BPLDAT[bpl] = chip.read(BPLPT[bpl]);
    BPLPT[bpl]++;
    
    // HRM: The parallel-to-serial conversion is triggered whenever
    // bitplane #1 is written, indicating the completion of all
    // bitplanes for that word (16 pixels). The MSB is output first,
    // and is, therefore, always on the left.
    if (bpl > 0)
      return true;
    
    for (int i = 0; i < 16; i++) {
      int val = 0;
      for (int d = depth() - 1; d >= 0; d--)
        val = (val << 1) | ((BPLDAT[d] >> i) & 1);
      bpldat[i] = val;
    }
    
    bpldatReady = 16;
    return true;
  }
  
  boolean fetchSprite(int vpos, int cycle) {
    if ((DMACON & DMAF_SPRITE) == 0)
      return false;

    if (vpos < VBlankEnd)
      return false;
      
    if (cycle < FirstSpriteCycle && cycle >= LastSpriteCycle)
      return false;
    
    cycle /= 2;
    
    int num = cycle / 2;
    int word = cycle % 2;

    if (SPRPT[num] < 0)
      return false;

    if (sprstat[num] == SpriteStatus.END)
      return false;

    int data = chip.read(SPRPT[num]);
    SPRPT[num]++;

    if (sprstat[num] == SpriteStatus.POSCTL) {
      if (word == 0)
        SPRPOS[num] = data;
      else
        SPRCTL[num] = data;
      return true;
    }
    
    if (sprstat[num] == SpriteStatus.DATA) {
      if (word == 0)
        SPRDATA[num] = data;
      else
        SPRDATB[num] = data;
      return true;
    }
    
    return false;
  }
  
  boolean fetchCopIns(int vpos, int cycle) {
    if ((DMACON & DMAF_COPPER) == 0)
      return false;

    return false;
  }
  
  void update() {
    bpldatReady = 0;
    
    loadPixels();
  
    for (int vpos = 0; vpos < MaxVertPos; vpos++) {
      for (int hpos = 0; hpos < MaxColorClk * 2; hpos++) {
        int cycle = hpos / 2;
        
        // on odd memory cycle fetch a word for custom chips 
        if (hpos % 2 == 1) {
          if (!fetchBitplane(vpos, cycle))
            if (!fetchSprite(vpos, cycle))
              fetchCopIns(vpos, cycle);
        }
        
        /* Found in UAE source code - DDFSTRT & DDFSTOP matching for:
         * - ECS: does not require DMA or DIW enabled,
         * - OCS: requires DMA and DIW enabled. */
        int ci = 0;
        
        if (bpldatReady > 0) {
          ci = bpldat[16 - bpldatReady];
          bpldatReady--;
        }

        if (vpos < VBlankEnd || hpos < HBlankEnd)
          continue;
          
        int x = hpos - HBlankEnd;
        int y = vpos - VBlankEnd;
        int c;
        
        if (vpos < vstart() || vpos >= vstop() || hpos < hstart() || hpos >= hstop())
          c = color(255, 204, 0);
        else 
          c = COLOR[ci];
          
        int w = OcsRasterWidth;
        int i = (y * w + x) * 2;   
        pixels[i] = c;
        pixels[i+1] = c;
        pixels[i+w] = c;
        pixels[i+w+1] = c;
      }
    }
  
    updatePixels();
  }
}
