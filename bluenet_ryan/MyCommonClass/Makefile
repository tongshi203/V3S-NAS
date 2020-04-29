.PHONY:


all: clean
	make -C src/mycommonlib CFG=DEBUG PWD=$(shell pwd)
	make -C src/mycommonlib CFG=RELEASE PWD=$(shell pwd)
	make -C src/mylibdvbpsi CFG=DEBUG PWD=$(shell pwd)
	make -C src/mylibdvbpsi CFG=RELEASE PWD=$(shell pwd)

clean:
	make -C src/mycommonlib clean
	make -C src/mylibdvbpsi clean
