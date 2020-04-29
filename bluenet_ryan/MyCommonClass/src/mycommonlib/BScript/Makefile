CC	 := arm-elf-gcc
AR	:=arm-elf-ar
# if define __FOR_MICROWIN_EMBED__ then work with MicroWindow
# if define __FOR_MY_OSD_EMBED__ then work with my OSD driver

FLAGS	 := -DMWPIXEL_FORMAT=MWPF_PALETTE -DHAVE_FILEIO -DHAVE_PNG_SUPPORT=1 -DHAVE_GIF_SUPPORT=1 \
-DLINUX=1 -DUNIX=1 -D__FOR_MY_OSD_EMBED__=1 -D_USE_MY_FLOAT_ -D_DEBUG=1

# If define CFG=DEBUG, then -D_DEBUG=1 is add
#set the default build environ
ifndef CFG
  CFG   :=RELEASE
endif

#setup output path lib name
ifeq ($(CFG),DEBUG)
    FLAGS    += -D_DEBUG=1 -D_MYDEBUG
	OUTLIBNAME	 := ../../lib/libbscriptD.a
	OBJOUTDIR	:=/root/work/BScript/debug
else
	OUTLIBNAME	 := ../../lib/libbscript.a
	OBJOUTDIR	:=/root/work/BScript/release
endif

INCLUDES := -I. \
	    -I$(EM85xx_ROOT)/thirdparties/microwindows/src/include \
	    -I$(EM85xx_ROOT)/kernelmodule/realmagichwl_userland \
	    -I$(EM85xx_ROOT)thirdparties/libpng -I/$(EM85xx_ROOT)/thirdparties/zlib \
	    -I../../include -I ../../include/BScript
ifdef DEBUG_BUILD	    
OBJFLAGS := -Wall -g -ffunction-sections -fdata-sections -o
OFLAGS	 := -Wall -ffunction-sections -fdata-sections --gc-sections -Wl,-elf2flt="-s16384"
else
OBJFLAGS := -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections -o
OFLAGS	 := -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections --gc-sections -Wl,-elf2flt="-s16384"
endif	
LIBDIR	 := -L$(EM85xx_ROOT)/thirdparties/microwindows/src/lib
OSDOBJ	 := $(EM85xx_ROOT)/kernelmodule/realmagichwl_userland/realmagichwl_userland_api.o
OBJS	:= blib.o bscript.o context.o gram.o lex.o module.o runtime.o BScriptEngine.o

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
	cp -u BScriptEngine.h ../../include/BScript
	cp -u blib.h ../../include/BScript

%.o:	%.cpp
	$(CC) -c $(FLAGS) $(INCLUDES) $(OBJFLAGS) $(OBJOUTDIR)/$@ $<
	$(AR) -r $(OUTLIBNAME) $(OBJOUTDIR)/$@

%.o:	%.c
	$(CC) -c $(FLAGS) $(INCLUDES) $(OBJFLAGS) $(OBJOUTDIR)/$@ $<
	$(AR) -r $(OUTLIBNAME) $(OBJOUTDIR)/$@

