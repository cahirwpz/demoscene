static Board board;

static int active = 0;
static int experiment = 0;

Random random = new Random();

Experiment[] experiments = {
  new DualPlay(),
  new RandomPlay(),
  new AdditivePlay(),
  new GenerationPlay(),
  new FromJSONPlay("wolfram.json"), // extracted from https://demonstrations.wolfram.com/Turmites/ 
  new FromJSONPlay("dumps.json"),
};

void setup() {
  size(512, 604);
  background(0);

  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);

  turmites = new ArrayList<Turmite>();
  board = new Board(128, 128);

  reset();
}

void status() {
  String status = experiments[experiment].status();

  fill(64);
  rect(2, 512, width-8, 88, 8);
  fill(255);
  text(status, 6, 528);
}

void draw() {
  board.simulate();
  status();
}

void reset() {
  turmites.clear();
  experiments[experiment].setup();
  board.reset();
}


void keyPressed() {
  if (key == 'r') {
    board.reset();
  } else if (key == 't') {
    board.nextTileClass();
  } else if (key == 'e') {
    experiment++;
    if (experiment >= experiments.length) {
      experiment = 0;
    }
    reset();
  } else {
    experiments[experiment].keyPressed();
  }
}

void mousePressed() {
  if (mouseX < 512 && mouseY < 512) {
    experiments[experiment].mousePressed(mouseX / 4, mouseY / 4);
  }
}
