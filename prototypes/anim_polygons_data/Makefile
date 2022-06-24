all: dancing.svg

docker:
	docker build -t anim-polygons .

dancing.svg: docker
	docker create --tty --name anim-polygons-env anim-polygons bash
	docker start anim-polygons-env
	docker cp docker-files/ anim-polygons-env:/root
	docker exec anim-polygons-env make -C docker-files
	docker cp anim-polygons-env:/root/docker-files/dancing.svg $@
	docker stop anim-polygons-env
	docker rm anim-polygons-env

clean:
	rm -f dancing.svg

.PHONY: all docker clean
