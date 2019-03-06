#! /usr/bin/make -f

builds := $(wildcard build*)

ifeq "$(builds)" ""
nobuild:
	@ echo "error: you don't seem to have any build directory (build*)" >&2
	@ echo "" >&2
	@ echo "You can configure one like this:" >&2
	@ echo "  mkdir build && cd build && ../configure [options...]" >&2
	@ echo "" >&2
	@ false
endif

#
#  all targets
#

all-targets := $(foreach b,$(builds),all-$b)

all: $(all-targets)

all-build%: build%
	@ echo -e "\n$B::$W Making into $<$N"
	@ $(MAKE) --no-print-directory -C $<

#
#  install targets
#

install-targets := $(foreach b,$(builds),install-$b)

install: $(install-targets)

install-build%: build%
	@ echo -e "\n$B::$W Installing from $<$N"
	@ $(MAKE) --no-print-directory -C $< install

#
#  uninstall targets
#

uninstall-targets := $(foreach b,$(builds),uninstall-$b)

uninstall: $(uninstall-targets)

uninstall-build%: build%
	@ echo -e "\n$B::$W Uninstalling from $<$N"
	@ $(MAKE) --no-print-directory -C $< uninstall

#
#  Coloring tools
#

B = \e[34;1m
W = \e[39;1m
N = \e[0m

.PHONY: nobuild all all-build% install install-build%
