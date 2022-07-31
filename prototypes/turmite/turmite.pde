static Board board;

static int active = 0;
static int experiment = 0;

Experiment[] experiments = {
  new DualPlay(), new RandomPlay()
};

void setup() {
  size(512, 604);
  background(0);
  
  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);
  
  board = new Board(128, 128);
  turmites = new ArrayList<Turmite>();
 
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
  board.reset();
  turmites.clear();
  experiments[experiment].setup();
}
  

void keyPressed() {
  if (key == 'r') {
    board.reset();
  } else if (key == 'p') {
    experiment--;
    if (experiment < 0) {
      experiment = experiments.length - 1;
    }
    reset();
  } else if (key == 'n') {
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
