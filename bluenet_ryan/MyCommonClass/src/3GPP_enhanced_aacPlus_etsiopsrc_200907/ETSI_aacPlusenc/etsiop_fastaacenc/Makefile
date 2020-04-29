#
# Makefile for aac encoder library
#

BASEDIR=../
include $(BASEDIR)/make.cfg 

INCDIR := -I. -I./src -I../etsioplib -I../etsiop_ffrlib -I../etsiop_bitbuf -I../etsiop_sbrenc  -I../etsiop_sbrenc/src -I../etsiop_resamplib -I../src

################## user section: insert objlist here #######
LIB = libaacenc.$(lib)

OBJS =  $(OBJDIR)/adj_thr.$(obj) \
	$(OBJDIR)/band_nrg.$(obj) \
	$(OBJDIR)/block_switch.$(obj) \
	$(OBJDIR)/grp_data.$(obj) \
	$(OBJDIR)/interface.$(obj) \
	$(OBJDIR)/ms_stereo.$(obj) \
	$(OBJDIR)/pre_echo_control.$(obj) \
	$(OBJDIR)/psy_configuration.$(obj) \
	$(OBJDIR)/psy_main.$(obj) \
	$(OBJDIR)/spreading.$(obj) \
	$(OBJDIR)/tns.$(obj) \
	$(OBJDIR)/tns_param.$(obj) \
	$(OBJDIR)/transform.$(obj) \
	$(OBJDIR)/fft.$(obj) \
	$(OBJDIR)/adj_thr.$(obj) \
	$(OBJDIR)/dyn_bits.$(obj) \
	$(OBJDIR)/line_pe.$(obj) \
	$(OBJDIR)/qc_main.$(obj) \
	$(OBJDIR)/quantize.$(obj) \
	$(OBJDIR)/sf_estim.$(obj) \
	$(OBJDIR)/stat_bits.$(obj) \
	$(OBJDIR)/bit_cnt.$(obj) \
	$(OBJDIR)/bitenc.$(obj) \
	$(OBJDIR)/aacenc.$(obj) \
	$(OBJDIR)/stprepro.$(obj) \
	$(OBJDIR)/channel_map.$(obj) \
	$(OBJDIR)/aac_ram.$(obj) \
	$(OBJDIR)/aac_rom.$(obj)

ifeq "$(MONO)" "1"
override CFLAGS+=-DMONO_ONLY
endif

include ../stdtargets.cfg

DATE   :=$(shell date +%Y-%m-%d)
TARGET  = FFR_aacenclib

