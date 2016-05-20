#
#	fx-9860g lib Makefile.
#

.PHONY: all clean fclean re install



#
#	Variables and configuration.
#

#  Tools
cc = sh3eb-elf-gcc
as = sh3eb-elf-as
ar = sh3eb-elf-ar
ob = sh3eb-elf-objcopy
wr = g1a-wrapper

# Output files
g1a = ginttest.g1a
bin = build/ginttest.bin
elf = build/ginttest.elf

#  Command-line options
cflags	= -m3 -mb -nostdlib -ffreestanding \
          -W -Wall -pedantic -std=c11 \
          -I . -isystem include
lib	= -lgcc -L. -lgint -lc



#
#	Source and object files.
#

# Gint library.
src-lib = crt0.c syscalls.s \
          gint.c gint_vbr.s gint_7705.c gint_7305.c \
          mpu.c keyboard.c screen.c display.c gray.c timer.c tales.c \
          bopti.c
hea-lib = 7305.h 7705.h gint.h \
          stdlib.h \
          mpu.h keyboard.h screen.h display.h gray.h timer.h tales.h
obj-lib = $(addprefix build/, $(addsuffix .o, $(src-lib)))
hdr-lib = $(addprefix include/, $(hea-lib))

#  Standard library.
src-std = setjmp.s string.c
hea-std = setjmp.h string.h ctype.h
obj-std = $(addprefix build/, $(addsuffix .o, $(src-std)))
hdr-std = $(addprefix include/, $(hea-std))

#  Test application.
src-app = ginttest.c
img-app = bitmap_opt.bmp swords.bmp sprites.bmp symbol.bmp symbol2.bmp \
          illustration.bmp
res-app = build/font.o $(addprefix build/, $(addsuffix .o, $(img-app)))


#
#	Building rules.
#

all: build libgint.a libc.a ginttest.g1a

build:
	mkdir -p build

libgint.a: $(obj-lib)
	$(ar) rcs libgint.a $(obj-lib)
	@ echo "\033[32;1mLibrary file size: "`stat -c %s libgint.a` \
		"bytes\033[0m"

libc.a: $(obj-std)
	$(ar) rcs libc.a $(obj-std)
	@ echo "\033[32;1mStandard file size: "`stat -c %s libc.a` \
		"bytes\033[0m"

ginttest.g1a: libgint.a $(src-app) $(res-app)
	$(cc) $(src-app) $(res-app) -T ginttest.ld -o $(elf) $(cflags) $(lib)
	$(ob) -R .comment -R .bss -O binary $(elf) $(bin)
	$(wr) $(bin) -o ginttest.g1a -i icon.bmp
	@ echo "\033[32;1mBinary file size: "`stat -c %s $(bin)`" bytes\033[0m"
	@ sh3eb-elf-objdump -h build/ginttest.elf



#
#	Resource management.
#

build/%.c.o: src/%.c $(hdr-lib) $(hdr-std)
	$(cc) $(cflags) -O2 -c $< -o $@

build/%.s.o: src/%.s
	$(as) -c $^ -o $@

build/%.bmp.o: resources/%.bmp
	fxconv $^ -o $@

build/font.o: resources/font.bmp
	fxconv --font $^ -o $@

# File gint.c should not be optimized... looks like attribute((interrupt_
# handler)) doesn't like it. (It could be a gint bug also, I should check.)
build/gint.c.o: src/gint.c $(hdr-lib) $(hdr-std)
	$(cc) $(cflags) -c $< -o $@

%.c.o: %.c $(hdr-lib) $(hdr-std)
	$(cc) $(cflags) -c $< -o $@
%.s.o: %.s
	$(as) -c $^ -o $@



#
#	Cleaning rules.
#

clean:
	@ rm -f $(obj-lib) $(obj-std) $(obj-app) $(res-app)
	@ rm -f $(bin) $(elf)
mrproper: clean
	@ rm -f build/*
	@ rm -f ginttest.g1a libc.a libgint.a
distclean: mrproper

re: distclean all



#
#	Installing shorthand.
#

install:
	usb-connector SEND ginttest.g1a ginttest.g1a fls0
