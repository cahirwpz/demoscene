# Anim-polygons data creation pipeline

Here's a little tutorial on how to generate it yourself:

### 1. Build an Docker Image
This step needs to be done only once, so don't worry if it takes a while...

```
docker build -t anim-polygons .
```

### 2. Run docker in interactive mode with bind mounted "shared" directory

```
docker run -v $(pwd)/shared:/root/shared -it anim-polygons /bin/bash
```



#### 2.1 Render scene

Use ```render.py``` script provided in ```shared/``` directory to render scene.

This script takes two positional arguments:
```anim-dir``` -> Directory for blender to output animation.
```scene``` -> Blender .scene file which is going to be rendered.

WARNING: This script removes ```anim-dir``` contents.

```
python3 render.py scene.blend anim/ 
```

Now you should have animation rendered and ready for the next step.

#### 2.2 Generate .svg

Use ```png2svg.py``` script provided in ```shared/``` directory to process rendered images into single svg file.

This script takes one positional argument:
```anim-dir``` -> Directory with previously rendered animation.

And one optional argument:
```--output OUTPUT.svg``` -> Output svg file name. (default is ```dancing.svg```)


```
python3 png2svg.py anim/
```

### 3 Exit docker

If you followed all the steps correctly you should end up with a valid ```dancing.svg``` file that you can drop into ```effects/anim-polygons/data``` directory.


### Closing remarks

If you want to create your own blender scene and use it in this pipeline, I highly advise you to use provided ```scene.blend``` file.
It contains all rendering settings and materials you need to successfully create your own ```dancing.svg``` file.
