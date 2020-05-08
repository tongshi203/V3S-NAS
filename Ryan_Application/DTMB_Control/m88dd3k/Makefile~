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


CC := arm-linux-gcc 
STRIP := arm-linux-strip

# Module Name
TARGET = TS_MXL241
# for DVB-C
SRCS = main.c MxL241.c Mxl_i2c.c MxL241SF_PhyCtrlApi.c MxL241SF_PhyCfg.c MxL241SF_OEM_Drv.c

#SRCDIRS	= include
#-l ./$(SRCDIRS)/

all: 
	$(CC) -lm $(SRCS) -o $(TARGET)
	$(STRIP) $(TARGET) 

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
