all: zopfli.so

zopfli.so:
	python setup.py build_ext --inplace

clean:
	@rm -vrf build
	@rm -vf zopfli.c
	@rm -vf *.so *.pyc *~

.PHONY: all clean
