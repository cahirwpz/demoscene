all: torus-u.png torus-v.png torus-light.png

torus-u.png: torus.pov
	povray -D Width=640 Height=480 +O$@ Declare=UMAP=1 $^

torus-v.png: torus.pov
	povray -D Width=640 Height=480 +O$@ Declare=VMAP=1 $^

torus-light.png: light.pov
	povray -D Width=640 Height=480 +O$@ $^

clean:
	rm -f *~ torus-*.png
