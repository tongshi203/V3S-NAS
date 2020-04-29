# setup compiler and linker
CC	:=g++
LD	:=g++
AR	:=ar

#setup modules name and output path
MODULE_NAME	:=libmycommon
TOP_DIR		:=$(PWD)

#seup source files
OBJ_FILES	:= MyString.o crc.o MyTime.o MyFile.o MyMemFile.o MyStdioFile.o myheap.o  myargparser.o MyThread.o\
	MySyncObj.o StatBpsHelper.o my_log_print.o my_bit_ring_buf.o mythreadpool.o my_map_file.o my_memory_block_mgr.o\
	MyArchive.o MyMapFile.o

#set the default build environ
ifndef CFG
  CFG	:=RELEASE
endif

ifeq ($(shell uname -p),x86_64)
	LIB_DIR=lib64
else
	LIB_DIR=lib
endif


#setup output path lib name
ifeq ($(CFG),DEBUG)
	OUT_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)D.a
	CPPFLAGS	+= -D_DEBUG -g -O0
else
	OUT_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME).a
	CPPFLAGS	+= -O3
endif

# currently, only support 32 bits
CPPFLAGS += -I. -pthread -fPIC

.PHONY: default cph echo_env

#all
all: echo_env default

echo_evn:
	@echo ----------------- $(CFG) -----------------------------------------

default:$(OBJ_FILES)
	ls -l $(OUT_LIB_NAME)
	cp -u *.h $(TOP_DIR)/include
	rm *.o
	@echo ===========================  The End =============================

cph:
	cp -u *.h $(TOP_DIR)/include

.cpp.o:
	@echo Compiling "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<
	@$(AR) -r $(OUT_LIB_NAME) $@
.c.o:
	@echo Compiling "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<
	@$(AR) -r $(OUT_LIB_NAME) $@

clean:
	rm -rf *.cpp~ *.h~ *.o
	rm -rf $(OUT_LIB_NAME)


