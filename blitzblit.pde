int WIDTH = 1024;
int HEIGHT = 1024;
int[] pal = {#000000, #ffffff};

PFont font;

// which one's drawn
boolean curr_bpl = false;

public enum Prompt {
  PRepl(">>"),
  PScriptRepl("S>"),
  PScriptEdit("E>"),
  PScriptNew("+>"),
  PScriptRunning("..");

  private final String str;

  private Prompt(String s){
    this.str = s;
  }

  final String getStr(){
    return this.str;
  }
}

Prompt cur_state = Prompt.PRepl;


// [0; 255]
// 164 looks really cool
// 130
// 150
// 218
// 220
// 222
// 64
// 62
// 160
// 126
// 190
// 174
// 188
// 186
int minterms = 170;

boolean blockdraw = false;
int[][] bpl0, bpl1;

String input_buf = "";
String output_buf = "";
int output_timeout = 0;

public static class Command {
  private Command(){}

  public static final class Load extends Command {
    public int slot;
    public Load(int i) {
      this.slot = i;
    }
    public String toString() {
      return "load " + slot;
    }    
  }

  public static final class Clear extends Command {
    public String toString() {
      return "clear";
    }    
  }

  public static final class SetMinterm extends Command {
    public int minterm;
    public SetMinterm(int i) {
      this.minterm = i;
    }
    public String toString() {
      return "setminterm " + minterm;
    }    
  }

  public static final class Wait extends Command {
    public int frames;
    public Wait(int i) {
      this.frames = i;
    }
    public String toString() {
      return "wait " + frames;
    }    
  }

  public static final class Pause extends Command {
    public String toString() {
      return "pause";
    }
  }

  public static final class Resume extends Command {
    public String toString() {
      return "resume";
    }    
  }

  public static final class SetBPM extends Command {
    public float bpm;
    public SetBPM(float bpm){
      this.bpm = bpm;
    }

    public String toString() {
      return "setbpm " + bpm;
    }
}

  public static final class WaitNSubdiv extends Command {
    public int subdivision;
    public WaitNSubdiv(int i) {
      this.subdivision = i;
    }

    public String toString() {
      return "waitnsubdiv " + subdivision;
    }
  }
}

ArrayList<Command> cmd_list = new ArrayList<Command>();
int cur_nextintr = 0;
int cur_ip = 0;
boolean blockscript = true;

int executeCommand(Command cmd) {
  if (cmd instanceof Command.Load){
    loadimage(((Command.Load)cmd).slot);
  } else if (cmd instanceof Command.Clear) {
    clearbpls();    
  } else if (cmd instanceof Command.SetMinterm) {
    minterms = ((Command.SetMinterm)cmd).minterm;
  } else if (cmd instanceof Command.Wait) {
    return ((Command.Wait)cmd).frames;
  } else if (cmd instanceof Command.Pause) {
    blockdraw = true;
  } else if (cmd instanceof Command.Resume) {
    blockdraw = false;
  } else if (cmd instanceof Command.SetBPM) {
    // TODO
  } else if (cmd instanceof Command.WaitNSubdiv) {
    // TODO
  } else {
    // TODO panic
  }

  return 0;
}

void executeLoop() {
  if (blockscript) {
    return;
  }

  if (frameCount < cur_nextintr)
    return;

  if (cur_ip >= cmd_list.size()) {
    blockscript = true;
    cur_state = Prompt.PScriptRepl;
    return;
  }
  
  while(cur_ip < cmd_list.size()) {
    int wait = executeCommand(cmd_list.get(cur_ip++));
    if (wait > 0) {
      cur_nextintr = frameCount + wait;
      return;
    }
  }
}

void settings() {
  size(WIDTH, HEIGHT);
}

void clearbpls() {
  for( int x = 0; x < WIDTH; x++)
    for(int y = 0; y < HEIGHT; y++){
      bpl0[x][y] = 0;
      bpl1[x][y] = 0;
    }      
}

int calcb(int minterms, int a, int b, int c) {
  int tmp = (a << 2) | (b << 1) | c;
  
  if(tmp > 7) return 0;
  
  int ret = (minterms & (1 << tmp)) != 0 ? 1 : 0;
  
  return ret; 
}

void loadimage() {
  loadimage(2);
}

void loadimage(int slot) {
  blockdraw = true;
  curr_bpl = true;
  
  PImage starting = loadImage("start"+slot+".png");
  starting.loadPixels();

  for (int x =0; x < starting.width; x++)
    for (int y=0; y < starting.height; y++) {
      float b = brightness(starting.get(x, y));
      
      if (b > 2 * 256.0 / 4) {
        bpl0[x][y] = 1;
      }
    }
  blockdraw = false;
}

void setup() {
  frameRate(50);
  surface.setTitle("BlitzBlit");
  bpl0 = new int[WIDTH][HEIGHT];
  bpl1 = new int[WIDTH][HEIGHT];
  
  font = createFont("Arial", 32);
  startStuff();
}

void draw() {

  executeLoop();

  int c_minterms = minterms;

  if (frameCount == output_timeout)
    output_buf = "";

  if(!blockdraw) {  
    for (int x = 1; x < WIDTH-1; x++)
      for (int y = 1; y < HEIGHT-1; y++)
        if (curr_bpl) {
          this.g.set(x, y, pal[bpl0[x][y]]);
          bpl1[x][y] = calcb(c_minterms, bpl0[x-1][y], bpl0[x+1][y+1], bpl0[x][y-1]); 
        }
        else {
          this.g.set(x, y, pal[bpl1[x][y]]);
          bpl0[x][y] = calcb(c_minterms, bpl1[x-1][y], bpl1[x+1][y+1], bpl1[x][y-1]);
        }
    curr_bpl = !curr_bpl;
  }

  textFont(font);
  text(output_buf, 10, 870);
  text(input_buf, 10 + 32*1.5, 900);
  text(cur_state.getStr(), 10, 900);
}

void startStuff() {
  clearbpls();
  loadimage(); 
  blockdraw = false;
}

void printMsg(String s) {
   output_buf = s;
   output_timeout = frameCount + (int)frameRate*3;
}

void saveScript(String path) {
  try {
    PrintWriter f = new PrintWriter(sketchPath("") + "/" + path, "UTF-8");
    f.println("//BlitzBlit v0.0.1");
    cmd_list.forEach((cmd) -> f.println(cmd));
    f.close();
    printMsg("Saved to " + path);
  }
  catch (Exception e) {
    printMsg("ERR!: " + e);
  }
}

void REPLprocessInput() {
  if(input_buf.length() == 0){
    printMsg("minterms = "+minterms);
    return;
  }

  if (input_buf.startsWith("dump")) {
    saveScript(input_buf.substring(4).trim());
    return;
  }

  switch (input_buf.charAt(0)) {
    case 'r':
      int tmp = 0;
      try {
        tmp = Integer.parseInt(input_buf.substring(1));
      } catch(NumberFormatException e) {
        tmp = (int)(Math.random()*255);
        tmp = tmp & (254);
      }
      if( tmp >= 0 && tmp <= 255)
        minterms = tmp;
      printMsg("Set minterms to "+minterms);
      break;

    case 's':
      cur_state = Prompt.PScriptNew;
      break;
    
    case 'q':
      exit();
      break;
  }
}

void interactRepl(char key, int keyCode) {
  if (key == CODED) {
    if (keyCode == CONTROL){
      startStuff();
      printMsg("Reloaded...");
    }  
    return;
  }
  
  if (key == '\n') {
    REPLprocessInput();
    input_buf = "";
  } else if (' ' <= key && key <= '~') {
    input_buf += key;
  } else if (keyCode == BACKSPACE) {
    input_buf = input_buf.substring(0, max(0, input_buf.length()-1));
  }   
}

void ScriptNewProcessInput() {
  if (input_buf.length() == 0) {
    cur_state = Prompt.PScriptRepl;
    return;
  }

  String[] parts = input_buf.split("\\s");

  if (parts.length == 0) {
    printMsg("ERR! zerolength");
    return;
  }

  switch (parts[0]) {
  case "clear":
    cmd_list.add(new Command.Clear());
    return;

  case "resume":
  case "start":
    cmd_list.add(new Command.Resume());
    return;

  case "pause":
    cmd_list.add(new Command.Pause());
    return;

  case "load":
    if (parts.length == 2) {
      try {
        int tmp = Integer.parseInt(parts[1]);
        cmd_list.add(new Command.Load(tmp));
      } catch(NumberFormatException e) {
        printMsg("ERR!");
        return;
      }
    }
    cmd_list.add(new Command.Load(2));
    return;

  case "setminterm":
  case "minterm":
    if (parts.length != 2) {
      printMsg("ERR!");
      return;
    }

    try {
      int tmp = Integer.parseInt(parts[1]);
      cmd_list.add(new Command.SetMinterm(tmp));
    } catch(NumberFormatException e) {
      printMsg("ERR!");
      return;
    }
    return;

  case "wait":
    if (parts.length != 2) {
      printMsg("ERR!");
      return;
    }
    try {
      int tmp = Integer.parseInt(parts[1]);
      cmd_list.add(new Command.Wait(tmp));
    } catch(NumberFormatException e) {
      printMsg("ERR!");
      return;
    }
    return;
  }
  printMsg("ERR!");
}

void interactScriptNew(char key, int keyCode) {
  if (key == CODED)
    return;

  if (key == '\n') {
    ScriptNewProcessInput();
    input_buf = "";
  } else if (Character.isLetterOrDigit(key) || Character.isWhitespace(key)) {
    input_buf += key;
  } else if (keyCode == BACKSPACE) {
    input_buf = input_buf.substring(0, max(0, input_buf.length()-1));
  }
}

void interactScriptRepl(char key, int keyCode) {
  switch (key) {
  case '\n':
    cur_state = Prompt.PRepl;
    break;

  case 'n':
    cmd_list = new ArrayList<Command>();
    cur_state = Prompt.PScriptNew;
    break;

  case 'r':
    blockscript = false;
    cur_ip = 0;
    cur_nextintr = 0;
    cur_state = Prompt.PScriptRunning;
    break;
  }

}

void keyPressed() {
  if (!keyPressed)
    return;

  switch (cur_state) {
    case PRepl:
      interactRepl(key, keyCode);
      break;
    case PScriptRepl:
      interactScriptRepl(key, keyCode);
      break;
    case PScriptEdit:
      break;
    case PScriptNew:
      interactScriptNew(key, keyCode);
      break;
    case PScriptRunning:
      blockscript = true;
      // blockdraw = true;
      cur_state = Prompt.PRepl;
      break;
  }
}
