# setup compiler and linker
CC	:=g++
LD	:=g++
AR	:=ar

#setup modules name and output path
MODULE_NAME	:=test

#seup source files
OBJ_FILES = test_main.o test_threadpool.o


#set the default build environ
ifndef CFG
  CFG	:=DEBUG
endif

#setup output path lib name
ifeq ($(CFG),DEBUG)
    TARGET_NAME	:=../bin/$(MODULE_NAME)D
else
    TARGET_NAME	:=../bin/$(MODULE_NAME)
endif

# get thirt part program build flags
ifeq ($(CFG),DEBUG)
    CPPFLAGS		+= -D_DEBUG -O0 -g -Wall
    LIB_DBG_EXT_NAME=D
else
    CPPFLAGS += -O3
endif

#my common class
CPPFLAGS		+= -I/usr/local/include/mycommon -pthread
MY_LINK_FLAGS 	+=-L/usr/local/lib64/mycommon -pthread
MY_LINK_FLAGS 	+=-ltinyxpath$(LIB_DBG_EXT_NAME)
MY_LINK_FLAGS 	+=-lmycommon$(LIB_DBG_EXT_NAME)

# MySQL
MY_LINK_FLAGS	+= -L/usr/lib64/mysql -lmysqlclient

MY_LINK_FLAGS += -lm -lpthread -lz -lrt


VPATH=../src

.PHONY: default,show_env

#all
all:	show_env default

#show build environ
show_env:
	@echo CFG= $(CFG)

default:$(OBJ_FILES)
	$(CC) -o $(TARGET_NAME) $(OBJ_FILES) $(MY_LINK_FLAGS)
	rm *.o
	@echo ===========================  The End =============================

.cpp.o:
	@echo [ Compiling ] "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<

.c.o:
	@echo [ Compiling ] "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<

clean:
	rm -rf *.cpp~ *.h~ *.o
	rm $(TARGET_NAME)

