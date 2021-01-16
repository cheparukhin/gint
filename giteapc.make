# giteapc: version=1 depends=Lephenixnoir/sh-elf-gcc,Lephenixnoir/fxsdk

-include giteapc-config.make

configure:
	@ mkdir -p build-fx && cd build-fx && ../configure --target=fx9860g
	@ mkdir -p build-cg && cd build-cg && ../configure --target=fxcg50

build:
	@ make all

install:
	@ make install

uninstall:
	@ make uninstall

.PHONY: configure build install uninstall
