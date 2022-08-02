all: dancing.svg

dancing.svg: anim/frame0000.png
	python3 png2svg.py --output $@ anim/
	
anim/frame0000.png: scene.blend
	test -d anim || mkdir anim
	rm -f anim/frame*.png
	xvfb-run --auto-servernum --server-args='-screen 0 1600x1024x16' \
		 blender --background $^ --render-output 'anim/frame####.png' \
		         --render-anim -noaudio

%: %.xz
	unxz --keep $^

clean:
	rm -f dancing.svg scene.blend
	rm -rf anim/

.PHONY: all clean
