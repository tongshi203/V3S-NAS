# setup compiler and linker
CC	:=g++
LD	:=g++
AR	:=ar

#setup modules name and output path
MODULE_NAME	:=libtinyxpath
TOP_DIR		:=/cyj/cyj/MyCommonClass

ifeq ($(shell uname -p),x86_64)
	LIB_DIR=lib64
else
	LIB_DIR=lib
endif

#seup source files
OBJ_FILES	:= action_store.o htmlutil.o lex_util.o node_set.o\
	tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o tokenlist.o\
    xml_util.o xpath_expression.o xpath_processor.o xpath_stack.o\
    xpath_static.o xpath_stream.o xpath_syntax.o

#set the default build environ
ifndef CFG
  CFG	:=RELEASE
endif

#setup output path lib name
ifeq ($(CFG),DEBUG)
	OUT_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)D.a
else
	OUT_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME).a
endif

# get thirt part program build flags
# wxWindows
ifeq ($(CFG),DEBUG)
	CPPFLAGS		+= -D_DEBUG -g -O0
endif

CPPFLAGS	+= -I$(TOP_DIR)/include -pthread

.PHONY: default,show_env

#all
all:	show_env default

#show build environ
show_env:
	@echo CFG= $(CFG)
	@echo OUT_LIB_NAME = $(OUT_LIB_NAME)

default:$(OBJ_FILES)
	ls -l $(OUT_LIB_NAME)
	rm *.o
	mkdir -p  $(TOP_DIR)/include/tinyxpath
	cp -u *.h $(TOP_DIR)/include/tinyxpath/
	@echo ===========================  The End =============================

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

