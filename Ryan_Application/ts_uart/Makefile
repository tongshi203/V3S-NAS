.SUFFIXES : .x .o .c .s

CC := arm-linux-gnueabihf-gcc

TARGET = ts_uart
SRCS := ts_uart.c
LIBS= -lc -lgcc -lc -lpthread

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
