# setup compiler and linker
CC	:=g++
LD	:=g++
AR	:=ar

#setup modules name and output path
TARGET_NAME=test

#seup source files
OBJ_FILES = testmain.o ../src/myrudp_socket.o ../src/myrudp_onepeerobj.o ../src/myrudp_con_helper_svr.o ../src/myrudp_packbuilder.o \
	../src/myrudp_packet_mgr.o ../src/myrudp_packet_sender.o ../src/myrudp_dbgprint.o \
	../src/myrudp_openssl.o ../src/myrudp_openssl_svr_helper.o 

OBJ_TEST_CLIENT = test_send.o ../src/myrudp_client.o ../src/myrudp_packet_mgr.o ../src/myrudp_packet_sender.o \
	../src/myrudp_dbgprint.o ../src/myrudp_openssl.o

OBJ_FILE_TESTECDH=testecdh.o ../src/myrudp_openssl.o ../src/myrudp_dbgprint.o

CPPFLAGS		+= -D_DEBUG -O0 -g -Wall

#my common class
CPPFLAGS		+= -I/usr/local/include/mycommon -pthread -I../src
CPPFLAGS += -D__MYRUDP_USE_OPENSSL__  
MY_LINK_FLAGS 	+=-L/usr/local/lib64/mycommon -pthread -lssl
MY_LINK_FLAGS 	+=-lmycommonD

MY_LINK_FLAGS += -lm -lpthread -lz -lrt



.PHONY: default,show_env,test_send

VPATH=../src

#all
all:	show_env default test_send
	rm -f core.*

#show build environ
show_env:
	@echo CFG= $(CFG)

default:$(OBJ_FILES)
	$(CC) -o $(TARGET_NAME) $(OBJ_FILES) $(MY_LINK_FLAGS)
	rm *.o
	@echo ===========================  The End =============================

test_send:$(OBJ_TEST_CLIENT)
	$(CC) -o test_send $(OBJ_TEST_CLIENT) $(MY_LINK_FLAGS)
	rm *.o

testecdh:$(OBJ_FILE_TESTECDH)
	$(CC) -o testecdh $(OBJ_FILE_TESTECDH) $(MY_LINK_FLAGS)
	rm *.o
.cpp.o:
	@echo [ Compiling ] "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<

.c.o:
	@echo [ Compiling ] "$<" ...
	@$(CC) -c $(CPPFLAGS) -o $@ $<

clean:
	rm -rf *.cpp~ *.h~ *.o
	rm -rf ../src/*.o
	rm -f $(TARGET_NAME)
	rm -f test_send
	rm -f core.*
	rm -f *.txt



