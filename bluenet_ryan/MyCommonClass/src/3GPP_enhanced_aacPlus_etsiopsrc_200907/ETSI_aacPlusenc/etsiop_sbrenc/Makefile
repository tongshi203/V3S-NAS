#
# Makefile for SBR encoder - library
#

BASEDIR=../
include $(BASEDIR)/make.cfg 

INCDIR := -I. -I../etsioplib -I../etsiop_ffrlib -I../etsiop_bitbuf -I../etsiop_fastaacenc -I../etsiop_fastaacenc/src -I../src

################## user section: insert objlist here #######
LIB = libsbrenc.$(lib)

OBJS =	$(OBJDIR)/code_env.$(obj) \
	$(OBJDIR)/env_est.$(obj) \
	$(OBJDIR)/env_bit.$(obj) \
	$(OBJDIR)/fram_gen.$(obj) \
	$(OBJDIR)/freq_sca.$(obj) \
	$(OBJDIR)/nf_est.$(obj) \
	$(OBJDIR)/qmf_enc.$(obj) \
	$(OBJDIR)/tran_det.$(obj) \
	$(OBJDIR)/mh_det.$(obj) \
	$(OBJDIR)/sbr_rom.$(obj) \
	$(OBJDIR)/sbr_ram.$(obj) \
	$(OBJDIR)/invf_est.$(obj) \
	$(OBJDIR)/sbr_misc.$(obj) \
	$(OBJDIR)/bit_sbr.$(obj) \
	$(OBJDIR)/sbr_main.$(obj) \
	$(OBJDIR)/ton_corr.$(obj) 

ifeq "$(MONO)" "1"
override CFLAGS+=-DMONO_ONLY
else
OBJS += $(OBJDIR)/ps_enc.$(obj) \
        $(OBJDIR)/ps_bitenc.$(obj) \
        $(OBJDIR)/hybrid.$(obj)
endif

include ../stdtargets.cfg

DATE  :=$(shell date +%Y-%m-%d)
TARGET = FFR_sbrenclib

