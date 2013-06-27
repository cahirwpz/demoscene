OBJ = quant

all:
	python setup.py build_ext --inplace

clean:
	@rm -vrf build
	@rm -vf $(OBJ).{html,c,o,so}
	@rm -vf *~

.PHONY: all
