int GRID_SIZE = 32;
int SPEED = 2;
String SKULLS_IMAGE = "skull_32x.png";
String BALL_IMAGE = "ball-poly.png";

// Features
float HIGHLIGHT_DISTANCE = 80.0;
boolean DISTANCE_SKULL_SIZE_FACTOR = true;
int HIGHLIGHT_FRAME_NUMBER = 7;
int MIN_BALL_SIZE = 10;
int MAX_BALL_SIZE = 40;
int BALL_SPRITE_FRAME_SIZE = 32;

PImage skullImage;
PImage ballImage;
SkullSprite [][] skullsGrid;
BouncingBall bball;

int gridCols;
int gridRows;

class SkullSprite {
  PVector position;
  PImage image;
  boolean highlighted;

  SkullSprite(PVector position_, PImage image_) {
    position = position_;
    image = image_;
    highlighted = false;
  }

  void show(int frame) {
    int frameToDisplay = highlighted ? HIGHLIGHT_FRAME_NUMBER : frame;
    copy(image, 0, 32 * frameToDisplay, 32, 32, Math.round(position.x), Math.round(position.y), 32, 32);
  }
}

class BouncingBall {
  PImage image;
  PVector position;
  PVector direction;
  float size;
  int sizeDirection;

  BouncingBall(PImage image_) {
    image = image_;
    position = new PVector(10, 10);
    direction = new PVector(1.5, 1);
    size = MIN_BALL_SIZE;
    sizeDirection = 1;
  }
  void draw() {
    int frame = frameCount%12;
    copy(image, BALL_SPRITE_FRAME_SIZE * frame, 0, BALL_SPRITE_FRAME_SIZE, BALL_SPRITE_FRAME_SIZE, round(position.x - size/2), round(position.y - size/2), round(size), round(size));
  }

  void setXdirection(float xDirection) {
    direction = new PVector(xDirection, direction.y);
  }
  void setYdirection(float yDirection) {
    direction = new PVector(direction.x, yDirection);
  }
}

// TODO Move it to the ball class
void updateBallPosition() {
  double timeFactor = abs(sin(frameCount/20.0)) * MAX_BALL_SIZE - MIN_BALL_SIZE;
  PVector position = bball.position;
  PVector direction = bball.direction;
  PVector nextPosition = new PVector(position.x + direction.x * SPEED, position.y + direction.y * SPEED);

  float nextSize = MIN_BALL_SIZE + (float)timeFactor;

  // Change direction is always for the smalles ball size,
  // Dose not look super good when ball is close to max size.
  if (nextPosition.x > width - MIN_BALL_SIZE || nextPosition.x < MIN_BALL_SIZE) {
    bball.setXdirection(-direction.x);
  }
  if (nextPosition.y > height - MIN_BALL_SIZE || nextPosition.y < MIN_BALL_SIZE) {
    bball.setYdirection(-direction.y);
  }
  bball.position = nextPosition;

  //
  int nextDirection = bball.size - nextSize < 0 ? -1 : 1;
  if ((nextDirection == -1) && (nextDirection != bball.sizeDirection)) {
    int skullGridX = floor(position.x / GRID_SIZE);
    int skullGridY = floor(position.y / GRID_SIZE);
    skullsGrid[skullGridY][skullGridX].highlighted = true;
  }

  bball.size = nextSize;
  bball.sizeDirection = nextDirection;
}

void skullsInit() {
  skullImage = loadImage(SKULLS_IMAGE);
  skullsGrid = new SkullSprite[gridCols][gridRows];
  for (int i = 0; i < gridCols; i++) {
    for (int j = 0; j < gridRows; j++) {
      skullsGrid[i][j] = new SkullSprite(new PVector(j * 32, i * 32), skullImage);
    }
  }
}

void ballsInit() {
  ballImage = loadImage(BALL_IMAGE);
  bball = new BouncingBall(ballImage);
}

void setup()
{
  size(320, 320);
  frameRate(30);
  gridCols = width / GRID_SIZE;
  gridRows = height / GRID_SIZE;
  skullsInit();
  ballsInit();
}


void draw() {
  background(0);
  updateBallPosition();
  for (int i = 0; i < gridCols; i++) {
    for (int j = 0; j < gridRows; j++) {
      SkullSprite skull = skullsGrid[i][j];
      PVector skullPosition = skull.position;
      float distance = dist(bball.position.x, bball.position.y, skullPosition.x, skullPosition.y);
      float ballSizeFactor = DISTANCE_SKULL_SIZE_FACTOR ? bball.size * 1.5 : 0;
      int frame = Math.round(map(distance + ballSizeFactor, 0, HIGHLIGHT_DISTANCE, 6, 1));
      skull.show(frame);
    }
  }
  bball.draw();
}
