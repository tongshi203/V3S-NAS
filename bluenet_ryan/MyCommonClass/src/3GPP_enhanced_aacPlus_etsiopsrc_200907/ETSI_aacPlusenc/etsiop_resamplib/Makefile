#
# Makefile for resampler library
#

BASEDIR=../
include $(BASEDIR)/make.cfg


INCDIR := -I. -I../etsioplib -I../etsiop_ffrlib


################## user section: insert objlist here #######
LIB = libFFR_resamp.$(lib)

OBJS =	$(OBJDIR)/downsample_FIR.o

ifeq "$(MONO)" "1"
override CFLAGS+=-DMONO_ONLY
endif

include $(BASEDIR)/stdtargets.cfg
