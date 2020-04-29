#
# Makefile for enhAacPlusEnc
#

DATE        := $(shell date +%Y-%m-%d)
MAKE         = gmake

BASEDIR=.
include $(BASEDIR)/make.cfg 

INCDIR := -I$(BASEDIR) -I$(BASEDIR)/etsioplib -I$(BASEDIR)/etsiop_ffrlib -I$(BASEDIR)/etsiop_bitbuf -I$(BASEDIR)/etsiop_fastaacenc -I$(BASEDIR)/etsiop_fastaacenc/src -I$(BASEDIR)/etsiop_resamplib -I$(BASEDIR)/etsiop_sbrenc -I$(BASEDIR)/etsiop_sbrenc/src -I$(BASEDIR)/3g_lib -I$(BASEDIR)/3g_lib/linux

ifeq "$(MONO)" "1"
override CFLAGS+=-DMONO_ONLY
TARGET = enhAacPlusEnc.mono
else
TARGET = enhAacPlusEnc
endif

ENCLIB = $(LIBPATH)/lib$(TARGET).$(lib)

LINKERFLAGS =  -L$(LIBPATH) -l$(TARGET) -L./3g_lib/linux -laudio -lisomediafile

SRC       =./src
OBJ       =./src
DIST      =internal/distribution.lst

MAINOBJS     = $(OBJDIR)/main.$(obj)
ALTMAINOBJS  = $(OBJDIR)/brswitchmain.$(obj)
OBJS         = $(OBJDIR)/mp4file.$(obj)
COMMONOBJS   =
sources      = $(SRC)/main.c \
	       $(SRC)/mp4file.c \
               $(SRC)/brswitchmain.c

all: enhAacPlusEnc enhAacPlusEnc_brswitch

enhAacPlusEnc: .depend $(MAINOBJS) ENCLIB
	$(CXX) -o $(TARGET) $(MAINOBJS) $(LINKERFLAGS) $(CFLAGS)

enhAacPlusEnc_brswitch: .depend $(ALTMAINOBJS) ENCLIB
	$(CXX) -o enhAacPlusEnc_brswitch $(ALTMAINOBJS) $(LINKERFLAGS)

.depend:
	@echo "Creating dependencies..."
	@echo "$(CFLAGS) $(INCDIR)"
	@for i in $(sources); do \
	($(CC_DEPEND) -MM $(CFLAGS) $(INCDIR) $$i >> tmp_depend); done
	@mv tmp_depend .depend

ETSIOP:
	cd etsioplib ; make WMOPS=$(WMOPS)

FFRLIB:
	make -C etsiop_ffrlib DEBUG=$(DEBUG) OPTIM=$(OPTIM) 

BITBUF: 
	make -C etsiop_bitbuf DEBUG=$(DEBUG) OPTIM=$(OPTIM) 

AACENCLIB:
	make -C etsiop_fastaacenc DEBUG=$(DEBUG) OPTIM=$(OPTIM) MONO=$(MONO)

RESAMPLIB:
	make -C etsiop_resamplib DEBUG=$(DEBUG) OPTIM=$(OPTIM) MONO=$(MONO)

SBRENCLIB:
	make -C etsiop_sbrenc DEBUG=$(DEBUG) OPTIM=$(OPTIM) MONO=$(MONO)

ENCLIB: $(OBJS) ETSIOP FFRLIB BITBUF AACENCLIB RESAMPLIB SBRENCLIB
	mkdir -p tmp-obj
	rm -f tmp-obj/*.$(obj)
	cd tmp-obj ; $(AR) x ../etsioplib/$(LIBPATH)/libetsiop.$(lib)
	cd tmp-obj ; $(AR) x ../etsiop_ffrlib/$(LIBPATH)/libffr.$(lib)
	cd tmp-obj ; $(AR) x ../etsiop_bitbuf/$(LIBPATH)/libbitbuf.$(lib)
	cd tmp-obj ; $(AR) x ../etsiop_fastaacenc/$(LIBPATH)/libaacenc.$(lib)
	cd tmp-obj ; $(AR) x ../etsiop_resamplib/$(LIBPATH)/libFFR_resamp.$(lib)
	cd tmp-obj ; $(AR) x ../etsiop_sbrenc/$(LIBPATH)/libsbrenc.$(lib)
	cd tmp-obj ; $(RM) ../$(ENCLIB)
	cp $(OBJS) tmp-obj
	cd tmp-obj ; $(AR) rcu ../$(ENCLIB) *.$(obj)
	$(RM) -r tmp-obj

clean:
	rm -f .depend
	rm -f $(MAINOBJS) $(OBJS) $(TARGET)
	make -C etsioplib         DEBUG=$(DEBUG) OPTIM=$(OPTIM) clean
	make -C etsiop_ffrlib     DEBUG=$(DEBUG) OPTIM=$(OPTIM) clean
	make -C etsiop_bitbuf     DEBUG=$(DEBUG) OPTIM=$(OPTIM) clean
	make -C etsiop_fastaacenc DEBUG=$(DEBUG) OPTIM=$(OPTIM) clean
	make -C etsiop_resamplib  DEBUG=$(DEBUG) OPTIM=$(OPTIM) clean
	make -C etsiop_sbrenc     DEBUG=$(DEBUG) OPTIM=$(OPTIM) clean

cleanall: clean

tags:
	etags *.c *.cpp *.h

.PHONY:	always
always:	
	@if (test -d $(LIBPATH)); then echo; else echo "building directory structure..."; $(MK_DIR) $(LIBPATH) 2> /dev/null; $(MK_DIR) $(OBJDIR) 2> /dev/null; fi

-include always

-include .depend

