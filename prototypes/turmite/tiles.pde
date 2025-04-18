class Tile {
  int ci; /* color index */
  int life;
  Turmite owner;

  void age() {
    life = life > 0 ? life - 1 : 0;
    if (life == 0) {
      reset();
    }
  }  

  void reset() {
    ci = 0;
    life = 0;
    owner = null;
  }

  void move(Turmite turmite) {
    ci = turmite.transition(ci);
    owner = turmite;
  }

  color colorOf() {
    if (ci == 0 || owner == null) return BGCOL;
    return FGCOL;
  }
}

class BasicTile extends Tile {
  void move(Turmite turmite) {
    super.move(turmite);
    life = ci > 0 ? 255 : 0;
  }

  color colorOf() {
    if (ci == 0 || owner == null) return BGCOL;
    return lerpColor(0, owner.palette[ci], life / 255.0);
  }
}

class HeatTile extends Tile {  
  void move(Turmite turmite) {
    super.move(turmite);
    life += 4;
    if (life > 255)
      life = 255;
  }

  color colorOf() {
    if (owner == null) return BGCOL;
    return palette[constrain(life, 0, 255)];
  }
}

class GenerationTile extends Tile {
  void move(Turmite turmite) {
    super.move(turmite);
    life = generation % 256;
  }

  color colorOf() {
    if (owner == null) return BGCOL;
    return palette[life];
  }
}
