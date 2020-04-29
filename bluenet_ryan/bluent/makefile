CC            = arm-linux-gnueabihf-gcc
CXX           = arm-linux-gnueabihf-g++
DEFS          = -DULE
CFLAGS        = -pipe -g -Wall -W -D_REENTRANT -fPIE $(DEFINES) $(DEFS)
CXXFLAGS      = -pipe -g -Wall -W -D_REENTRANT -fPIE $(DEFINES) $(DEFS)
INCPATH       = -I.
OUT_NAME      := bluenet

ifeq ($(CFG),DEBUG)
        OUT_NAME	:= $(OUT_NAME)D
        CFLAGS          += -D _DEBUG -g -O0
        CXXFLAGS	+= -D _DEBUG -g -O0
endif


all:default
default : main.o DVBDSMCC_IP_MPE.o CUDPDataPort.o CV4l2DataPort.o
	@echo -----------------------"<" Compile $(OUT_NAME) with $(DEFS) ">"--------------------
	$(CXX) -o $(OUT_NAME) main.o DVBDSMCC_IP_MPE.o CUDPDataPort.o CV4l2DataPort.o
	@echo ===========================  The End =============================

main.o: main.cpp CUDPDataPort.h \
		CV4l2DataPort.h \
		DVBDSMCC_IP_MPE.h \
		tspacket.h \
		mytype.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

DVBDSMCC_IP_MPE.o: DVBDSMCC_IP_MPE.cpp mytype.h \
		DVBDSMCC_IP_MPE.h \
		tspacket.h \
		dvbpsitablesdefine.h \
		dvbdescriptorsdefine.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o DVBDSMCC_IP_MPE.o DVBDSMCC_IP_MPE.cpp

CUDPDataPort.o: CUDPDataPort.cpp CUDPDataPort.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CUDPDataPort.o CUDPDataPort.cpp

CV4l2DataPort.o: CV4l2DataPort.cpp CV4l2DataPort.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CV4l2DataPort.o CV4l2DataPort.cpp

clean:
	rm *.o $(OUT_NAME)
