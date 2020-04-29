#
# Makefile for fractional arithmetic library
#

BASEDIR=../
include $(BASEDIR)/make.cfg

INCDIR := -I. -I.. -I../etsioplib

################## user section: insert objlist here #######
LIB = libffr.$(lib)


OBJS =	$(OBJDIR)/transcendent.$(obj) \
	$(OBJDIR)/vector.$(obj) \
	$(OBJDIR)/dsp_fft32x32s.$(obj) \
	$(OBJDIR)/intrinsics.$(obj) \
	$(OBJDIR)/transcendent_enc.$(obj)

include ../stdtargets.cfg

DATE   :=$(shell date +%Y-%m-%d)
TARGET  = ffrlib
