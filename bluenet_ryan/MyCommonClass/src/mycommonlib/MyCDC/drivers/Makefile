CPP	= arm-elf-gcc
CC	= arm-elf-gcc
CFLAGS	= -c
WAR 	= -Wall
INCLUDE = -I../ -I../include -I$(EM85xx_ROOT)/kernelmodule/realmagichwl_kernelland \
	  -I ../../../../cyj/include
CFLAGS	+= -O3  -DMWPIXEL_FORMAT=MWPF_PALETTE $(INCLUDE) $(WAR)
#CFLAGS	+= -g  -DMWPIXEL_FORMAT=MWPF_PALETTE $(INCLUDE) $(WAR) -D_DEBUG

SRCS	= scr_quasarosd.cpp genmem.c #nofont.c genfont.c
OBJS	= scr_quasarosd.o genmem.o #nofont.o genfont.o

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
