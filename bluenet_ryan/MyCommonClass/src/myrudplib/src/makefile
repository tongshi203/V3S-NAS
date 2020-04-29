# setup compiler and linker
CC	:=g++
LD	:=g++
AR	:=ar

#setup modules name and output path
MODULE_NAME	:=libmyrudp
TOP_DIR		:=/cyj/cyj/MyCommonClass

#seup source files
SVR_OBJ_FILES = myrudp_socket.o myrudp_onepeerobj.o myrudp_con_helper_svr.o myrudp_packbuilder.o \
	myrudp_packet_mgr.o myrudp_packet_sender.o myrudp_dbgprint.o myrudp_client.o

CLI_OBJ_FILES = myrudp_dbgprint.o myrudp_client.o myrudp_packet_mgr.o myrudp_packet_sender.o

#####################################################################
# using OpenSSL or not
CPPFLAGS += -D__MYRUDP_USE_OPENSSL__
SVR_OBJ_FILES += myrudp_openssl.o myrudp_openssl_svr_helper.o
CLI_OBJ_FILES += myrudp_openssl.o


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
	OUT_SVR_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)svrD.a
	OUT_CLI_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)cliD.a
	OUT_CLI_LIB_NAME_TP	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)clitpD.a
	CPPFLAGS	+= -D_DEBUG -g -O0
else
	OUT_SVR_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)svr.a
	OUT_CLI_LIB_NAME	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)cli.a
	OUT_CLI_LIB_NAME_TP	:=$(TOP_DIR)/$(LIB_DIR)/$(MODULE_NAME)clitp.a
	CPPFLAGS	+= -O3
endif

# currently, only support 32 bits
CPPFLAGS 	+= -I. -pthread -Wall
CPPFLAGS	+= -I$(TOP_DIR)/include


HEADER_FILES_COPY = myrudp_client_drv.h myrudp_cmd_value.h myrudp_event.h myrudp_server_drv.h

.PHONY: svrlib echo_env update_header

#all
all: echo_env svrlib clientlib update_header clientlib_tp rm_obj
	@rm -f *.o

update_header:
	mkdir -p $(TOP_DIR)/include/myrudp
	cp -u $(HEADER_FILES_COPY) $(TOP_DIR)/include/myrudp

echo_evn:
	@echo ----------------- $(CFG) -----------------------------------------

rm_obj:
	@echo "Clean all object files"
	@rm -f *.o


clientlib_tp:
	@rm -f *.o
	@$(CC) -c $(CPPFLAGS) -D__TEST_USING_THREAD_POOL__ -o myrudp_dbgprint.o myrudp_dbgprint.cpp
	@$(CC) -c $(CPPFLAGS) -D__TEST_USING_THREAD_POOL__ -o myrudp_client.o myrudp_client.cpp
	@$(CC) -c $(CPPFLAGS) -D__TEST_USING_THREAD_POOL__ -o myrudp_packet_mgr.o myrudp_packet_mgr.cpp
	@$(CC) -c $(CPPFLAGS) -D__TEST_USING_THREAD_POOL__ -o myrudp_packet_sender.o myrudp_packet_sender.cpp
	@$(CC) -c $(CPPFLAGS) -D__TEST_USING_THREAD_POOL__ -o myrudp_openssl.o myrudp_openssl.cpp
	@$(AR) -r $(OUT_CLI_LIB_NAME_TP) $(CLI_OBJ_FILES)
	ls -l $(OUT_CLI_LIB_NAME_TP)
	@echo ===========================  The End =============================

clientlib:$(CLI_OBJ_FILES)
	@$(AR) -r $(OUT_CLI_LIB_NAME) $(CLI_OBJ_FILES)
	ls -l $(OUT_CLI_LIB_NAME)
	@echo ===========================  The End =============================

svrlib:$(SVR_OBJ_FILES)
	@$(AR) -r $(OUT_SVR_LIB_NAME) $(SVR_OBJ_FILES)
	ls -l $(OUT_SVR_LIB_NAME)
	@echo ===========================  The End =============================

.cpp.o:
	@echo Compiling "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<

.c.o:
	@echo Compiling "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<

clean:
	rm -f *.cpp~ *.h~ *.o
	rm -f $(OUT_SVR_LIB_NAME)
	rm -f $(OUT_CLI_LIB_NAME)


