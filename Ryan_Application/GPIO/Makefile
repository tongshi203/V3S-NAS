.SUFFIXES : .x .o .c .s

CC := arm-linux-gnueabihf-gcc

TARGET = GPIO_Ctl
SRCS := GPIO_Ctl.c

all: 
	$(CC) -static $(SRCS) -o $(TARGET)

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
