#/**************************************************************************
# *	Author     : yuanlii@29bbs.net			_________________  *
# *	Date       : 04/01/2005 15:40:41		|    | This file | *
# *	File name  : Makefile				| vi |	powered  | *
# *	Description: The Make file for libylib.a	|____|___________| *
# *			it's the library file of my own			   *
# **************************************************************************/

CC = arm-elf-gcc
AR = arm-elf-ar cru
RM = rm -f
CFLAGS = -c -O2

LIBOBJS = atof.o strtod.o
LIBFILE = ../../lib/libmyfloat.a

.PHONY: all clean

all:	$(LIBFILE)

clean:
	$(RM) $(LIBOBJS)

#----------------------------real targets---------------------------#
%.o :	%.c
	$(CC) $(CFLAGS) $<

$(LIBFILE): $(LIBOBJS)
	$(AR) $@ $^
