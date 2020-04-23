#
# -= Makefile for module compile =-
#
# Usage:
# . Name this file as "Makefile";
#   Put it in the same directory as module's source code.
# . Change the ROOT definition, if necessary;
#   Make it point to the root of the software tree.
# . Define the Module Name in MOD as you like;
#   There should be NO duplicated module names in the whole project.
# . List all files want to be compiled in ASMS and/or SRCS;
#   Including files, such as header files, must NOT be listed here.
# . List all library files this module may depends on in LIBS.
# . Give a new name in SMOD, if want to include sub-directories;
#   Write the linkage information at the end of file.
#
.SUFFIXES : .x .o .c .s


CC := arm-linux-gnueabihf-gcc 


# Module Name
TARGET = TS_DTMB_Control
# for DTMB
SRCS = main.c m88dd3k.c fe_dd3k_i2c.c mt_fe_dmd_dd3k.c mt_fe_tn_tc6800.c

#SRCDIRS	= include
#-l ./$(SRCDIRS)/

all: 
	$(CC) -lm $(SRCS) -o $(TARGET)

clean:
	rm -f *.o 
	rm -f *.x 
	rm -f *.flat
	rm -f *.map
	rm -f temp
	rm -f *.img
	rm -f $(TARGET)	
	rm -f *.gdb
	rm -f *.bak
