
# gws - Gramado Window Server.
# 2020 - Created by Fred Nora.

# todo:
# Os aplicativos em /init/core ainda usam PE, assim como o base kernel.
# É desejável que isso mude.

VERSION = 0
PATCHLEVEL = 1
SUBLEVEL = 0
EXTRAVERSION = 
#NAME = GWS

# Make variables (CC, etc...)
AS	= as
LD	= ld
CC	= gcc
AR	= ar
MAKE	= make
NASM	= nasm
OBJCOPY	= objcopy
OBJDUMP	= objdump
LEX	= flex
YACC	= bison
PERL	= perl
PYTHON	= python
PYTHON2	= python2
PYTHON3	= python3
RUBY	= ruby


# usermode cross compiler flags.
user_ccflags = \
	-Wall \
	-Wundef \
	-Wmissing-prototypes \
	-Wno-uninitialized \
	-Wno-format-security \
	-Wno-trigraphs \
	-Werror=strict-prototypes \
	-Werror=implicit-function-declaration \
	-Werror=return-type \
	-std=gnu89 \
	-m64 \
	-s \
	-static \
	-fcommon \
	-fgnu89-inline \
	-finline-functions \
	-fshort-wchar \
	-ffreestanding \
	-fleading-underscore \
	-fstrict-overflow \
	-nostdlib \
	-nodefaultlibs \
	-nostdinc \
	-fno-builtin \
	-fno-stack-protector \
	-fno-strict-aliasing \
	-fno-PIE \
	-fno-pie \
	-fno-omit-frame-pointer \
	-fno-optimize-sibling-calls    
	
	
	
	
	

	


# usermode linker flags.
user_ldflags = -m elf_x86_64 -T x86_64/link_r0.ld


#todo
#path longo. vamos mudar  as libs de lugar para gramado/libs.

LIBC              = ../../../com/lib/rtl/include/
LIBCOBJ           = ../../../com/lib/rtl/obj
WS_INCLUDES       = include/ 
WS_INCLUDES_LOCAL = .


#
# Objects
#


#head
myObjects = crt0.o main.o rtl.o \
ctype.o stdio.o stdlib.o string.o conio.o socket.o signal.o unistd.o \
termios.o ioctl.o fcntl.o    

myObjects += archive_basics1.o

myObjects += archive_basics2.o

myObjects += archive_fx.o    

myObjects += archive_extra.o    

myObjects += wm.o   



.PHONY: all all-c gws-link finalize clean

all: main.o all-c gws-link finalize clean
	@echo "Ok?"

main.o:

	$(CC)  -c  main.c     $(user_ccflags) -I $(LIBC) -I . -o main.o

all-c:

#basics1
	$(CC)  -c  bitblt.c    $(user_ccflags) -I $(LIBC) -I . -o bitblt.o
	$(CC)  -c  char.c      $(user_ccflags) -I $(LIBC) -I . -o char.o
	$(CC)  -c  dtext.c     $(user_ccflags) -I $(LIBC) -I . -o dtext.o
	$(LD) -r -o archive_basics1.o  bitblt.o char.o dtext.o 


	$(CC)  -c  line.c      $(user_ccflags) -I $(LIBC) -I . -o line.o
	$(CC)  -c  rect.c      $(user_ccflags) -I $(LIBC) -I . -o rect.o
	$(CC)  -c  window.c    $(user_ccflags) -I $(LIBC) -I . -o window.o
	$(CC)  -c  gws.c       $(user_ccflags) -I $(LIBC) -I . -o gws.o
	$(CC)  -c  comp.c      $(user_ccflags) -I $(LIBC) -I . -o comp.o
	$(CC)  -c  bmp.c       $(user_ccflags) -I $(LIBC) -I . -o bmp.o
	$(CC)  -c  connect.c   $(user_ccflags) -I $(LIBC) -I . -o connect.o
	$(CC)  -c  button.c    $(user_ccflags) -I $(LIBC) -I . -o button.o
	$(LD) -r -o archive_basics2.o \
line.o rect.o window.o gws.o comp.o bmp.o connect.o button.o

#fx
	$(CC)  -c  grprim.c    $(user_ccflags) -I $(LIBC) -I . -o grprim.o
	$(CC)  -c  sprite.c    $(user_ccflags) -I $(LIBC) -I . -o sprite.o
	$(CC)  -c  demos.c     $(user_ccflags) -I $(LIBC) -I . -o demos.o
	$(LD) -r -o archive_fx.o  grprim.o sprite.o demos.o 

#extra
	$(CC)  -c  xxxinput.c  $(user_ccflags) -I $(LIBC) -I . -o xxxinput.o
	$(CC)  -c  font.c      $(user_ccflags) -I $(LIBC) -I . -o font.o
	$(LD) -r -o archive_extra.o  xxxinput.o font.o 


	$(CC)  -c  wm.c   $(user_ccflags) -I $(LIBC) -I include/ -I .  -o wm.o


#
# == imported objects =============================
#


	# rtl/
	cp $(LIBCOBJ)/crt0.o .
	cp $(LIBCOBJ)/rtl.o .

	cp $(LIBCOBJ)/ctype.o .
	cp $(LIBCOBJ)/stdio.o .
	cp $(LIBCOBJ)/stdlib.o .
	cp $(LIBCOBJ)/string.o .
	cp $(LIBCOBJ)/conio.o .
	cp $(LIBCOBJ)/unistd.o .
	cp $(LIBCOBJ)/socket.o .
	cp $(LIBCOBJ)/signal.o .
	cp $(LIBCOBJ)/termios.o .
	cp $(LIBCOBJ)/ioctl.o .
	cp $(LIBCOBJ)/fcntl.o .


gws-link:
	$(LD) $(user_ldflags) -o GWSSRV.BIN $(myObjects)
#	$(LD) -m elf_i386 -T ../link.ld -o GWSSRV.BIN $(myObjects)  

finalize:
	cp GWSSRV.BIN ../../bin

clean:
	-rm *.o
	-rm GWSSRV.BIN 
	
	
