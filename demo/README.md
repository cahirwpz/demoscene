Demo Engine
===

## Command line parameters

Only available in **debug mode**.

Demo accepts following parameters (all optional): `FIRST/N,LAST/N,LOOP/S,SHOWFRAME/S`

* `FIRST` - the number of first frame to be played (*default: 0*)
* `LAST` - the number of last frame to be played (*default: specified by timeline*)
* `LOOP` - don't quit, just play specified range of frames in a loop (*default: false*)
* `SHOWFRAME` - render layer of extra information including **frame per second** and **frame number**.
* `TIMELINE` - print demo timeline and quit.

Invocation examples:

* `demo FIRST 200 LAST 400 LOOP` - plays **[200, 400]** range of frames in a loop,
* `demo SHOWFRAME` - plays the demo once showing frames per second counter and frame counter.

## Keys

Only available in **debug mode**.

* `ESCAPE` - quit the demo immediately
* `LEFT ARROW` - fast forward by 1 second,
* `RIGHT ARROW` - rewind by 1 second,
* `UP ARROW` - fast forward by 10 seconds,
* `DOWN ARROW` - rewind by 10 seconds,
* `SPACE` - pause the demo.


## Bindings

To expose variables and functions to be used in the **timeline** the coder needs to mark them in C code as `CALLBACK` or `PARAMETER`. For instance:

```
PARAMETER(PixBufT *, Texture, NULL);
PARAMETER(int, PositionX, 0);

CALLBACK(RenderEffect) {
  /*
   Some C code to render the effect.
   May use "frame" variable to learn about timing.
   */
}
```

Which will make `"Texture"`, `"PositionX"` parameters available for modifications, and `"RenderEffect"` callable from outside (namely [JSON] timeline configuration).

After marking every desired function and variable one needs to generate symbol tables using `gen-bindings.py` tool:

```
$ gen-bindings.py demo.c > demo.syms
```

Finally one needs to include the symbol table somewhere at the end of demo source file.

```
#include "demo.syms"
```

Note that `PARAMETER` and `CALLBACK` macros define variable and function statically, so they are only visible within the same compilation unit.

### Callbacks

Callback function signature is given below. It takes an argument of `Frame` structure.

```
typedef struct Frame {
  int number;
  int first, last;
} FrameT;

typedef void (*TimeFuncT)(FrameT *frame);
```

`Frame` structure stores a range of frames (from `first` to `last`) counted from the beginning of the demo, when the callback is called. `number` is the number of current frame counted from `first` frame.

### Parameters

## Configuration

Demo configuration is stored in a text file in [JSON] format. The path to configuration file is specified by `const char *DemoConfigPath` variable.

[JSON]: (http://en.wikipedia.org/wiki/JSON)

### Basic settings

Specify demo run mode, iff *true* then demo is run in **debug mode**.

```
"debug": boolean
```

Specify the last frame of demo, after which the demo will quit:

```
"end-frame": integer
```

Specify music properties - beats per minute and path to ``WAVE`` file:

```
"music": {
  "bpm": float,
  "file": string
}
```

Specify loading screen image and palette:

```
"load": {
  "image": string,
  "palette": string
}
```

### Resources

A [JSON] object that stores the description of all predefined resources like: *image*, *palette*, *3d object*, etc. 

A key corresponds to the name of a resource, and a value to the information how to establish the resource.

Example:

```
"resources": {
    "txt-img": { "type": "image", "path": "data/texture.8" },
    "txt-pal": { "type": "palette", "path": "data/texture.pal" }
}
```

#### Known resources

##### Image

```
string: { "type": "image", "path": string },
```

##### Palette

```
string: { "type": "palette", "path": string },
```

### Timeline

Describes how the effects in the demo are ordered in time.
