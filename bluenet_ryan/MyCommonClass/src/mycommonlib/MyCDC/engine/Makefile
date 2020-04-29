CPP	= arm-elf-gcc
CC	= arm-elf-gcc
CFLAGS	= -c
WAR 	= -Wall
INCLUDE = -I.. \
	  -I../include \
	  -I ../../../../cyj/include \
	  -I$(EM85xx_ROOT)/kernelmodule/realmagichwl_kernelland
CFLAGS	+= -O3 -DMWPIXEL_FORMAT=MWPF_PALETTE $(INCLUDE) $(WAR)
#CFLAGS	+= -g  -DMWPIXEL_FORMAT=MWPF_PALETTE $(INCLUDE) $(WAR) -D_DEBUG

SRCS	= devopen.c devdraw.c devclip.c devrgn.c devarc.c devpoly.c devimage.c\
	  devlist.c error.c devfont.c devpal8.c mylibb.c devimg2.cpp
OBJS	= devopen.o devdraw.o devclip.o devrgn.o devarc.o devpoly.o devimage.o\
	  devlist.o error.o devfont.o devpal8.o mylibb.o devimg2.o

# file deletion command
RM	= rm -f

#-----------------------------PHONY TARGETS---------------------------------#
.PHONY: all clean rebuild objs

all	: objs

clean	:
	$(RM) $(OBJS)

rebuild	: clean all

objs	: $(OBJS)

#-----------------------------REAL  TARGETS---------------------------------#
%.o	: %.c
	$(CC) $(CFLAGS) -o $@ $<
%.o	: %.cpp
	$(CPP) $(CFLAGS) -o $@ $<
devpal8.o	: devpal8.c
	$(CC) $(CFLAGS) -o $@ $<
