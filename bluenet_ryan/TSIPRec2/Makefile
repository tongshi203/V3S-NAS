# setup compiler and linker
CC	:=arm-linux-gnueabihf-gcc
LD	:=arm-linux-gnueabihf-g++
AR	:=arm-linux-gnueabihf-ar

#setup modules name and output path
MODULE_NAME	:=libtsiprec2

#seup source files
OBJ_FILES	:= BaseFileCombiner.o BitArrayObject.o DecoderThread.o DirectroyHelp.o DVBEPGReceiver.o \
				DVBFileReceiver.o FileDelayEventDispatcher.o FileObject.o FileWriterThread.o HugeFile.o \
				IPFileMendHelper.o IPRDrvAPI.o IPRecSvr_i.o Lzhuf.o MB_OneFile.o StdAfx.o TSDB_Rec.o TSDBFileSystem.o \
				UDPDataPortLinux.o UDPRecThread.o UnCmpMgr.o UnCompressObj.o unlzss32.o

#setup output path lib name
LIBFLAGS += -Lsdk/lib -lpthread
ifeq ($(CFG),DEBUG)
	OUT_LIB_NAME	:=bin/$(MODULE_NAME)D.so
	CPPFLAGS	+= -D _DEBUG -g -O0
	LIBFLAGS	+= -lmycommonD
else
	OUT_LIB_NAME	:=bin/$(MODULE_NAME).so
	LIBFLAGS	+= -lmycommon
	STRIP_CMD = arm-linux-gnueabihf-strip $(OUT_LIB_NAME)
endif

CPPFLAGS += -I. -pthread -Isdk/inc -fPIC
LIBFLAGS += -pthread -fPIC

.PHONY: default makesdk

#all
all:default

default:$(OBJ_FILES)
	@echo ------------------------------"<" Linking $(CFG) ">"----------------------------
	$(LD) -o $(OUT_LIB_NAME) -shared $(OBJ_FILES)  $(LIBFLAGS)
	rm *.o
	$(STRIP_CMD)
	@echo ===========================  The End =============================

.cpp.o:
	@echo ------------------------------ $(CFG) ----------------------------
	@echo Compiling "$<" ...
	$(CC) -c $(CPPFLAGS) -o $@ $<
.c.o:
	@echo ------------------------------- $(CFG) ---------------------------
	@echo Compiling "$<" ...
	$(CC) -c $(CPPFLAGS) -o $@ $<

clean:
	rm -rf *.cpp~ *.h~ *.o *.~h *.~cpp
	rm -rf $(MODULE_NAME)

makesdk:
	tar jcfh demo.tar.bz2 demo_api
