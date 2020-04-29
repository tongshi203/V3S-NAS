CPP	= arm-elf-gcc
CC	= arm-elf-gcc
CFLAGS	= -c
WAR 	= -Wall
#CFLAGS += -O2 -fomit-frame-pointer
INCLUDE = -I../include -I$(EM85xx_ROOT)/thirdparties/microwindows/src/include \
	  -I ../../../../cyj/include
CFLAGS	+= -O3 -DMWPIXEL_FORMAT=MWPF_PALETTE $(INCLUDE) $(WAR)
#CFLAGS	+= -g -DMWPIXEL_FORMAT=MWPF_PALETTE $(INCLUDE) $(WAR) -D_DEBUG

SRCS	= wingdi.c mygdiobjs.cpp winfont.c
OBJS	= wingdi.o mygdiobjs.o winfont.o

TARGET  = y

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
mygdiobjs.o	: mygdiobjs.cpp
	$(CPP) $(CFLAGS) -o $@ $<
