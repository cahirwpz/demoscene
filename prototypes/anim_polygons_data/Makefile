SCENE ?= cock.blend
OUTPUT ?= $(SCENE:.blend=.svg)

all: $(OUTPUT)

docker:
	docker build -t anim-polygons .

$(OUTPUT): docker
	docker create --tty --name anim-polygons-env anim-polygons bash
	docker start anim-polygons-env
	docker cp docker-files/ anim-polygons-env:/root
	docker exec anim-polygons-env bash -c "cd docker-files && ./convert.sh run $(SCENE)"
	docker cp anim-polygons-env:/root/docker-files/$(OUTPUT) $@
	docker stop anim-polygons-env
	docker rm anim-polygons-env

clean:
	-docker stop anim-polygons-env
	-docker rm anim-polygons-env
	rm -f $(OUTPUT)

.PHONY: all docker clean
