#***************************************************************************
# *	Author     : yuanlii@29bbs.net			_________________  *
# *	Date       : 1/20/2005 14:47:22			|    | This file | *
# *	File name  : Makefile				| vi |	powered  | *
# *	Description: Makefile for libyobjs		|____|___________| *
# *									   *
# ***************************************************************************
#
CC	= arm-elf-gcc
CPP	= arm-elf-g++
AR	= arm-elf-ar
CFLAGS	= -O3
INCLUDE	= -Iinclude -I../../../cyj/include/
CFLAGS	+= $(INCLUDE)
WARS	= -Wall
CFLAGS	+= $(WARS)

SUBDIRS	= ./engine	\
	  ./cdcimplement\
	  ./drivers

GDILIB	= ../../lib/libmycdc.a
# add objs here
GDIOBJS	= drivers/scr_quasarosd.o\
	  drivers/genmem.o	\
	  engine/devopen.o	\
	  engine/devdraw.o	\
	  engine/devclip.o	\
	  engine/devrgn.o	\
	  engine/devarc.o	\
	  engine/devpoly.o	\
	  engine/devimage.o	\
	  engine/devlist.o	\
	  engine/error.o	\
	  engine/devfont.o	\
	  engine/devpal8.o	\
	  engine/mylibb.o	\
	  engine/devimg2.o	\
	  cdcimplement/wingdi.o	\
	  cdcimplement/mygdiobjs.o \
	  cdcimplement/winfont.o

# GNU make command
MAKE	= make
# file deletion command
RM	= rm -f

#-----------------------------PHONY TARGETS---------------------------------#
.PHONY: all clean uninstall rebuild tags lib

all:
	@$(MAKE) -C engine/
	@$(MAKE) -C cdcimplement/
	@$(MAKE) -C drivers/
	@$(MAKE) lib
#	@echo "[31mWill recover the libiary file![m"

clean:
	@$(MAKE) -C engine clean
	@$(MAKE) -C cdcimplement clean
	@$(MAKE) -C drivers clean

uninstall:
	$(RM) $(GDILIB)

rebuild:clean all


lib:	$(GDILIB)
	cp include/mygdiobjs.h ../../include/mycdc/

#-----------------------------REAL  TARGETS---------------------------------#
$(GDILIB)	: $(GDIOBJS)
	$(AR) cru $@ $^ 
	ranlib $@

%.o	: %.cpp
	$(CPP) $(CFLAGS) -c -o $@ $<
