CC	 := gcc
AR	:= ar

FLAGS	 := -DLINUX=1 -DUNIX=1
TOP_DIR=$(PWD)

# If define CFG=DEBUG, then -D_DEBUG=1 is add
#set the default build environ
ifndef CFG
  CFG   :=RELEASE
endif

ifeq ($(shell uname -p),x86_64)
    LIB_DIR=lib64
else
    LIB_DIR=lib
endif



INCLUDES := -I. -I$(TOP_DIR)/include

#setup output path lib name
ifeq ($(CFG),DEBUG)
    FLAGS    += -D_DEBUG=1 -D_MYDEBUG -g -O0
    OUTLIBNAME	:= $(TOP_DIR)/$(LIB_DIR)/libmydvbpsiD.a
    OBJOUTDIR	:=/tmp/libmydvbpsi/debug
    OBJFLAGS := -Wall
    OFLAGS	 := -Wall
else
    OUTLIBNAME	:= $(TOP_DIR)/$(LIB_DIR)/libmydvbpsi.a
    OBJOUTDIR	:=/tmp/libmydvbpsi/release
    OBJFLAGS := -Wall -O3
    OFLAGS	 := -Wall -O3
endif

# support 32 bits only
# FLAGS += -m32

LIBDIR	 := -L../../lib
OBJS	:= bitstream.o dvb_crc.o dvbdescriptors.o tspacketdemux.o pespacket.o dvbpsitables.o dvbpsiscanprogramhelper.o tsmpeg4bropmthelper.o\
 tspacketencapsulator.o pespacketencapsulator.o psitablegenerator.o tsvodpsi_tablegenerator.o dvbpsitable_ip_mpe.o\
 dvbpsi_ip_mpe_generator.o

#---------------------------------------------------------#
.PHONY:	default images clean rebuild tags MakeDirs
default:MakeDirs $(OUTLIBNAME)

MakeDirs:
	mkdir -p $(OBJOUTDIR)
	rm -f $(OBJOUTDIR)/*.o

clean:
	rm $(OBJOUTDIR)/*.o $(OUTLIBNAME) $(OBJOUTDIR)/*.gdb -f

rebuild:clean default

ags:
	ctags *.cpp *.h


all:	$(OUTLIBNAME)

$(OUTLIBNAME):	$(OBJS)
	ls $(OUTLIBNAME) -l
	mkdir -p $(TOP_DIR)/include/mydvbpsi
	cp *.h $(TOP_DIR)/include/mydvbpsi
	rm -f $(OBJOUTDIR)/*.o

%.o:	%.cpp
	@echo "[ Compiling ] " $<
	@$(CC) -c $(FLAGS) $(INCLUDES) $(OBJFLAGS) -o $(OBJOUTDIR)/$@ $<
	@$(AR) -r $(OUTLIBNAME) $(OBJOUTDIR)/$@

