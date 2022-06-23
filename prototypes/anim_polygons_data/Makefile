all: dancing.svg

docker:
	docker build -t anim-polygons .

dancing.svg:
	docker run --rm anim-polygons cat $@ > $@

clean:
	rm -f dancing.svg


.PHONY: all docker clean
