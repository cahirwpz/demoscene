Demo Engine
===

## Command line parameters

Only available in **debug mode**.

Demo accepts following parameters (all optional): `FIRST/N,LAST/N,LOOP/S,SHOWFRAME/S`

* `FIRST` - the number of first frame to be played (*default: 0*)
* `LAST` - the number of last frame to be played (*default: specified by timeline*)
* `LOOP` - don't quit, just play specified range of frames in loop (*default: false*)
* `SHOWFRAME` - render layer of extra information including **frame per second** and **frame number**.
* `TIMELINE` - print demo timeline and quit.

Invocation examples:

* `demo FIRST 200 LAST 400 LOOP` - plays the range of frames **[200, 400]** in a loop
* `demo SHOWFRAME` - plays the demo once showing frames per second counter and frame counter

## Keys

Only available in **debug mode**.

* `ESCAPE` - quit the demo immediately
* `LEFT ARROW` - fast forward by 1 second,
* `RIGHT ARROW` - rewind by 1 second,
* `UP ARROW` - fast forward by 10 seconds,
* `DOWN ARROW` - rewind by 10 seconds,
* `SPACE` - pause the demo.

## Configuration

Demo configuration is stored in a text file in [JSON](http://en.wikipedia.org/wiki/JSON) format. The path to configuration file is specified by `const char *DemoConfigPath` variable.

### Basic settings

Specify demo run mode:

```
"debug": boolean
```

Specify last frame of a demo:

```
"end-frame": integer
```

Specify beats per minute and path to ``WAVE`` file with music:

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

TODO

### Timeline

TODO
