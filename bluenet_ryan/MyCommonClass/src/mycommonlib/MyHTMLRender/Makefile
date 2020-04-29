CC	 := arm-elf-gcc
AR	:=arm-elf-ar
# if define __FOR_MICROWIN_EMBED__ then work with MicroWindow
# if define __FOR_MY_OSD_EMBED__ then work with my OSD driver

FLAGS	 := -DMWPIXEL_FORMAT=MWPF_PALETTE -DHAVE_FILEIO -DHAVE_PNG_SUPPORT=1 -DHAVE_GIF_SUPPORT=1 \
-DLINUX=1 -DUNIX=1 -D__FOR_MY_OSD_EMBED__=1


# If define CFG=DEBUG, then -D_DEBUG=1 is add
#set the default build environ
ifndef CFG
  CFG   :=RELEASE
endif

#setup output path lib name
ifeq ($(CFG),DEBUG)
    FLAGS    += -D_DEBUG=1 -D_MYDEBUG
	OUTLIBNAME	 := ../../lib/libmyhtmlrenderD.a
	OBJOUTDIR	:=/root/work/myhtmlrender/debug
else
	OUTLIBNAME	 := ../../lib/libmyhtmlrender.a
	OBJOUTDIR	:=/root/work/myhtmlrender/release
endif

INCLUDES := -I. \
	    -I$(EM85xx_ROOT)/thirdparties/microwindows/src/include \
	    -I$(EM85xx_ROOT)/kernelmodule/realmagichwl_userland \
	    -I$(EM85xx_ROOT)thirdparties/libpng -I/$(EM85xx_ROOT)/thirdparties/zlib \
	    -I../../include -I ../../include/mycdc
ifdef DEBUG_BUILD	    
OBJFLAGS := -Wall -g -ffunction-sections -fdata-sections -o
OFLAGS	 := -Wall -ffunction-sections -fdata-sections --gc-sections -Wl,-elf2flt="-s16384"
else
OBJFLAGS := -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections -o
OFLAGS	 := -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections --gc-sections -Wl,-elf2flt="-s16384"
endif	
LIBDIR	 := -L$(EM85xx_ROOT)/thirdparties/microwindows/src/lib
OSDOBJ	 := $(EM85xx_ROOT)/kernelmodule/realmagichwl_userland/realmagichwl_userland_api.o
OBJS	:= MyHTMLRender.o HTMLElements.o HTMLParser.o HTMLRender.o HTMLTokenizer.o MyCompoundFileObj.o

#---------------------------------------------------------#
.PHONY:	default images clean rebuild tags MakeDirs
default:MakeDirs $(OUTLIBNAME)

MakeDirs:
	mkdir -p $(OBJOUTDIR)
clean:
	rm $(OBJOUTDIR)/*.o $(OUTLIBNAME) $(OBJOUTDIR)/*.gdb -f

rebuild:clean default

tags:
	ctags *.cpp *.h


all:	$(OUTLIBNAME)

$(OUTLIBNAME):	$(OBJS)
	ls $(OUTLIBNAME) -l
	cp MyHTMLRender.h ../../include/MyHTMLRender
	cp ASPFunction.h ../../include/MyHTMLRender

%.o:	%.cpp
	$(CC) -c $(FLAGS) $(INCLUDES) $(OBJFLAGS) $(OBJOUTDIR)/$@ $<
	$(AR) -r $(OUTLIBNAME) $(OBJOUTDIR)/$@

