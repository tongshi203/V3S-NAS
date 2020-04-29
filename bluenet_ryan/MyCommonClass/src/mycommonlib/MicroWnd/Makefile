CC	 := arm-elf-gcc
AR	:=arm-elf-ar
FLAGS	 := -DMWPIXEL_FORMAT=MWPF_PALETTE -DHAVE_FILEIO -DHAVE_PNG_SUPPORT=1 -DHAVE_GIF_SUPPORT=1 \
-DLINUX=1 -DUNIX=1 -D__FOR_MICROWIN_EMBED__=1
INCLUDES := -I. \
	    -I$(EM85xx_ROOT)/thirdparties/microwindows/src/include \
	    -I$(EM85xx_ROOT)/kernelmodule/realmagichwl_userland \
	    -I$(EM85xx_ROOT)thirdparties/libpng -I/$(EM85xx_ROOT)/thirdparties/zlib \
	    -I../../include -I ../../include/MicroWnd
OBJFLAGS := -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections -o
OFLAGS	 := -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections --gc-sections -Wl,-elf2flt="-s16384"
LIBDIR	 := -L$(EM85xx_ROOT)/thirdparties/microwindows/src/lib
LIBS	 := -lmwin -lmwinlib -lmwengine -lmwdrivers -lmwfonts -lmwimages #-lm
OSDOBJ	 := $(EM85xx_ROOT)/kernelmodule/realmagichwl_userland/realmagichwl_userland_api.o
OUTLIBNAME	 := ../../lib/libmicrowndobjs.a
OBJOUTDIR	:=/root/work/micrownd
OBJS	:= mywnd.o mygdiobjs.o mwin_image.o

#---------------------------------------------------------#
.PHONY:	default images clean rebuild tags
default:$(OUTLIBNAME)

clean:
	rm $(OBJOUTDIR)/*.o $(OUTLIBNAME) $(OBJOUTDIR)/*.gdb -f

rebuild:clean default

tags:
	ctags *.cpp *.h


all:	$(OUTLIBNAME)

$(OUTLIBNAME):	$(OBJS)
	ls $(OUTLIBNAME) -l

%.o:	%.cpp
	$(CC) -c $(FLAGS) $(INCLUDES) $(OBJFLAGS) $(OBJOUTDIR)/$@ $<
	$(AR) -r $(OUTLIBNAME) $(OBJOUTDIR)/$@

