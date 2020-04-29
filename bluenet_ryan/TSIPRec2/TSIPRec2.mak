# C++BuilderX Version: 1.0.0.1786
# Generated GNU Makefile
# Do not modify, as your changes will be lost on re-export

# User Defined Variables:
# End User Defined Variables

# Start of configurations
# If the user specified no configuration on the command line, set a default:
ifndef CFG
  CFG := Debug Build
endif
ifeq ($(CFG),Debug Build)
  BUILD_DIR := ../../../../../root/work/TSIPRec2/Debug_Build
  Debug Build := 1
endif
ifeq ($(CFG),Release Build)
  BUILD_DIR := ../../../../../root/work/TSIPRec2/Release_Build
  Release Build := 1
endif
ifneq ($(CFG),Debug Build)
  ifneq ($(CFG),Release Build)
    $(error An incorrect configuration was specified)
  endif
endif
# End of configurations

ifeq ($(CFG),Debug Build)
  # Default build target if none specified:
default: builddir ../../../../../root/work/TSIPRec2/Debug_Build/TSIPRec2.so

all: default

../../../../../root/work/TSIPRec2/Debug_Build/BaseFileCombiner.o: BaseFileCombiner.cpp
	g++ -c -o $(BUILD_DIR)/BaseFileCombiner.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  BaseFileCombiner.cpp

../../../../../root/work/TSIPRec2/Debug_Build/BitArrayObject.o: \
         BitArrayObject.cpp
	g++ -c -o $(BUILD_DIR)/BitArrayObject.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  BitArrayObject.cpp

../../../../../root/work/TSIPRec2/Debug_Build/DecoderThread.o: \
         DecoderThread.cpp
	g++ -c -o $(BUILD_DIR)/DecoderThread.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  DecoderThread.cpp

../../../../../root/work/TSIPRec2/Debug_Build/DirectroyHelp.o: \
         DirectroyHelp.cpp
	g++ -c -o $(BUILD_DIR)/DirectroyHelp.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  DirectroyHelp.cpp

../../../../../root/work/TSIPRec2/Debug_Build/DVBFileReceiver.o: \
         DVBFileReceiver.cpp
	g++ -c -o $(BUILD_DIR)/DVBFileReceiver.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  DVBFileReceiver.cpp

../../../../../root/work/TSIPRec2/Debug_Build/FileObject.o: FileObject.cpp
	g++ -c -o $(BUILD_DIR)/FileObject.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  FileObject.cpp

../../../../../root/work/TSIPRec2/Debug_Build/HugeFile.o: HugeFile.cpp
	g++ -c -o $(BUILD_DIR)/HugeFile.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  HugeFile.cpp

../../../../../root/work/TSIPRec2/Debug_Build/IPFileMendHelper.o: IPFileMendHelper.cpp
	g++ -c -o $(BUILD_DIR)/IPFileMendHelper.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  IPFileMendHelper.cpp

../../../../../root/work/TSIPRec2/Debug_Build/IPRDrvAPI.o: IPRDrvAPI.cpp
	g++ -c -o $(BUILD_DIR)/IPRDrvAPI.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  IPRDrvAPI.cpp

../../../../../root/work/TSIPRec2/Debug_Build/IPRecSvr_i.o: IPRecSvr_i.cpp
	g++ -c -o $(BUILD_DIR)/IPRecSvr_i.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  IPRecSvr_i.cpp

../../../../../root/work/TSIPRec2/Debug_Build/Lzhuf.o: Lzhuf.cpp
	g++ -c -o $(BUILD_DIR)/Lzhuf.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  Lzhuf.cpp

../../../../../root/work/TSIPRec2/Debug_Build/MB_OneFile.o: MB_OneFile.cpp
	g++ -c -o $(BUILD_DIR)/MB_OneFile.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  MB_OneFile.cpp

../../../../../root/work/TSIPRec2/Debug_Build/StdAfx.o: StdAfx.cpp
	g++ -c -o $(BUILD_DIR)/StdAfx.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  StdAfx.cpp

../../../../../root/work/TSIPRec2/Debug_Build/TSDB_Rec.o: TSDB_Rec.cpp
	g++ -c -o $(BUILD_DIR)/TSDB_Rec.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  TSDB_Rec.cpp

../../../../../root/work/TSIPRec2/Debug_Build/TSDBFileSystem.o: TSDBFileSystem.cpp
	g++ -c -o $(BUILD_DIR)/TSDBFileSystem.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  TSDBFileSystem.cpp

../../../../../root/work/TSIPRec2/Debug_Build/UDPDataPortLinux.o: \
         UDPDataPortLinux.cpp
	g++ -c -o $(BUILD_DIR)/UDPDataPortLinux.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UDPDataPortLinux.cpp

../../../../../root/work/TSIPRec2/Debug_Build/UDPRecThread.o: \
         UDPRecThread.cpp
	g++ -c -o $(BUILD_DIR)/UDPRecThread.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UDPRecThread.cpp

../../../../../root/work/TSIPRec2/Debug_Build/UnCmpMgr.o: UnCmpMgr.cpp
	g++ -c -o $(BUILD_DIR)/UnCmpMgr.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UnCmpMgr.cpp

../../../../../root/work/TSIPRec2/Debug_Build/UnCompressObj.o: UnCompressObj.cpp
	g++ -c -o $(BUILD_DIR)/UnCompressObj.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UnCompressObj.cpp

../../../../../root/work/TSIPRec2/Debug_Build/unlzss32.o: unlzss32.cpp
	g++ -c -o $(BUILD_DIR)/unlzss32.o -g2 -O0 -D _DEBUG=1 -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  unlzss32.cpp

../../../../../root/work/TSIPRec2/Debug_Build/TSIPRec2.so: ../../../../../root/work/TSIPRec2/Debug_Build/BaseFileCombiner.o ../../../../../root/work/TSIPRec2/Debug_Build/BitArrayObject.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/DecoderThread.o ../../../../../root/work/TSIPRec2/Debug_Build/DirectroyHelp.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/DVBFileReceiver.o ../../../../../root/work/TSIPRec2/Debug_Build/FileObject.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/HugeFile.o ../../../../../root/work/TSIPRec2/Debug_Build/IPFileMendHelper.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/IPRDrvAPI.o ../../../../../root/work/TSIPRec2/Debug_Build/IPRecSvr_i.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/Lzhuf.o ../../../../../root/work/TSIPRec2/Debug_Build/MB_OneFile.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/StdAfx.o ../../../../../root/work/TSIPRec2/Debug_Build/TSDB_Rec.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/TSDBFileSystem.o ../../../../../root/work/TSIPRec2/Debug_Build/UDPDataPortLinux.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/UDPRecThread.o ../../../../../root/work/TSIPRec2/Debug_Build/UnCmpMgr.o \
         ../../../../../root/work/TSIPRec2/Debug_Build/UnCompressObj.o ../../../../../root/work/TSIPRec2/Debug_Build/unlzss32.o \
        
	g++ -shared -o $(BUILD_DIR)/TSIPRec2.so -lpthread ../../../../../root/work/TSIPRec2/Debug_Build/BaseFileCombiner.o ../../../../../root/work/TSIPRec2/Debug_Build/BitArrayObject.o ../../../../../root/work/TSIPRec2/Debug_Build/DecoderThread.o ../../../../../root/work/TSIPRec2/Debug_Build/DirectroyHelp.o ../../../../../root/work/TSIPRec2/Debug_Build/DVBFileReceiver.o ../../../../../root/work/TSIPRec2/Debug_Build/FileObject.o ../../../../../root/work/TSIPRec2/Debug_Build/HugeFile.o ../../../../../root/work/TSIPRec2/Debug_Build/IPFileMendHelper.o ../../../../../root/work/TSIPRec2/Debug_Build/IPRDrvAPI.o ../../../../../root/work/TSIPRec2/Debug_Build/IPRecSvr_i.o ../../../../../root/work/TSIPRec2/Debug_Build/Lzhuf.o ../../../../../root/work/TSIPRec2/Debug_Build/MB_OneFile.o ../../../../../root/work/TSIPRec2/Debug_Build/StdAfx.o ../../../../../root/work/TSIPRec2/Debug_Build/TSDB_Rec.o ../../../../../root/work/TSIPRec2/Debug_Build/TSDBFileSystem.o ../../../../../root/work/TSIPRec2/Debug_Build/UDPDataPortLinux.o ../../../../../root/work/TSIPRec2/Debug_Build/UDPRecThread.o ../../../../../root/work/TSIPRec2/Debug_Build/UnCmpMgr.o ../../../../../root/work/TSIPRec2/Debug_Build/UnCompressObj.o ../../../../../root/work/TSIPRec2/Debug_Build/unlzss32.o /mnt/f/cyj/MyCommonClass/Linux/lib/MyCommToolsGNCD.a

builddir:
	-mkdir -p ..
	-mkdir -p ../root
	-mkdir -p ../root/work
	-mkdir -p ../root/work/TSIPRec2
	-mkdir -p ../root/work/TSIPRec2/Debug_Build

clean:
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/BaseFileCombiner.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/BaseFileCombiner.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/BitArrayObject.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/BitArrayObject.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/DecoderThread.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/DecoderThread.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/DirectroyHelp.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/DirectroyHelp.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/DVBFileReceiver.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/DVBFileReceiver.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/FileObject.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/FileObject.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/HugeFile.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/HugeFile.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/IPFileMendHelper.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/IPFileMendHelper.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/IPRDrvAPI.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/IPRDrvAPI.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/IPRecSvr_i.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/IPRecSvr_i.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/Lzhuf.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/Lzhuf.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/MB_OneFile.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/MB_OneFile.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/StdAfx.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/StdAfx.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/TSDB_Rec.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/TSDB_Rec.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/TSDBFileSystem.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/TSDBFileSystem.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UDPDataPortLinux.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UDPDataPortLinux.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UDPRecThread.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UDPRecThread.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UnCmpMgr.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UnCmpMgr.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UnCompressObj.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/UnCompressObj.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/unlzss32.o
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/unlzss32.d
	-rm ../../../../../root/work/TSIPRec2/Debug_Build/TSIPRec2.so

endif
ifeq ($(CFG),Release Build)
  # Default build target if none specified:
default: builddir ../../../../../root/work/TSIPRec2/Release_Build/TSIPRec2.so

all: default

../../../../../root/work/TSIPRec2/Release_Build/BaseFileCombiner.o: BaseFileCombiner.cpp
	g++ -c -o $(BUILD_DIR)/BaseFileCombiner.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  BaseFileCombiner.cpp

../../../../../root/work/TSIPRec2/Release_Build/BitArrayObject.o: \
         BitArrayObject.cpp
	g++ -c -o $(BUILD_DIR)/BitArrayObject.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  BitArrayObject.cpp

../../../../../root/work/TSIPRec2/Release_Build/DecoderThread.o: \
         DecoderThread.cpp
	g++ -c -o $(BUILD_DIR)/DecoderThread.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  DecoderThread.cpp

../../../../../root/work/TSIPRec2/Release_Build/DirectroyHelp.o: \
         DirectroyHelp.cpp
	g++ -c -o $(BUILD_DIR)/DirectroyHelp.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  DirectroyHelp.cpp

../../../../../root/work/TSIPRec2/Release_Build/DVBFileReceiver.o: \
         DVBFileReceiver.cpp
	g++ -c -o $(BUILD_DIR)/DVBFileReceiver.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  DVBFileReceiver.cpp

../../../../../root/work/TSIPRec2/Release_Build/FileObject.o: \
         FileObject.cpp
	g++ -c -o $(BUILD_DIR)/FileObject.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  FileObject.cpp

../../../../../root/work/TSIPRec2/Release_Build/HugeFile.o: HugeFile.cpp
	g++ -c -o $(BUILD_DIR)/HugeFile.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  HugeFile.cpp

../../../../../root/work/TSIPRec2/Release_Build/IPFileMendHelper.o: \
         IPFileMendHelper.cpp
	g++ -c -o $(BUILD_DIR)/IPFileMendHelper.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  IPFileMendHelper.cpp

../../../../../root/work/TSIPRec2/Release_Build/IPRDrvAPI.o: \
         IPRDrvAPI.cpp
	g++ -c -o $(BUILD_DIR)/IPRDrvAPI.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  IPRDrvAPI.cpp

../../../../../root/work/TSIPRec2/Release_Build/IPRecSvr_i.o: IPRecSvr_i.cpp
	g++ -c -o $(BUILD_DIR)/IPRecSvr_i.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  IPRecSvr_i.cpp

../../../../../root/work/TSIPRec2/Release_Build/Lzhuf.o: Lzhuf.cpp
	g++ -c -o $(BUILD_DIR)/Lzhuf.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  Lzhuf.cpp

../../../../../root/work/TSIPRec2/Release_Build/MB_OneFile.o: MB_OneFile.cpp
	g++ -c -o $(BUILD_DIR)/MB_OneFile.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  MB_OneFile.cpp

../../../../../root/work/TSIPRec2/Release_Build/StdAfx.o: StdAfx.cpp
	g++ -c -o $(BUILD_DIR)/StdAfx.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  StdAfx.cpp

../../../../../root/work/TSIPRec2/Release_Build/TSDB_Rec.o: TSDB_Rec.cpp
	g++ -c -o $(BUILD_DIR)/TSDB_Rec.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  TSDB_Rec.cpp

../../../../../root/work/TSIPRec2/Release_Build/TSDBFileSystem.o: TSDBFileSystem.cpp
	g++ -c -o $(BUILD_DIR)/TSDBFileSystem.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  TSDBFileSystem.cpp

../../../../../root/work/TSIPRec2/Release_Build/UDPDataPortLinux.o: \
         UDPDataPortLinux.cpp
	g++ -c -o $(BUILD_DIR)/UDPDataPortLinux.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UDPDataPortLinux.cpp

../../../../../root/work/TSIPRec2/Release_Build/UDPRecThread.o: \
         UDPRecThread.cpp
	g++ -c -o $(BUILD_DIR)/UDPRecThread.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UDPRecThread.cpp

../../../../../root/work/TSIPRec2/Release_Build/UnCmpMgr.o: UnCmpMgr.cpp
	g++ -c -o $(BUILD_DIR)/UnCmpMgr.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UnCmpMgr.cpp

../../../../../root/work/TSIPRec2/Release_Build/UnCompressObj.o: UnCompressObj.cpp
	g++ -c -o $(BUILD_DIR)/UnCompressObj.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  UnCompressObj.cpp

../../../../../root/work/TSIPRec2/Release_Build/unlzss32.o: unlzss32.cpp
	g++ -c -o $(BUILD_DIR)/unlzss32.o -MD -I/usr/include -I/usr/include/g++-3 -I/mnt/f/cyj/MyCommonClass/Linux/include -I/mnt/f/cyj/dvb/TSIPRec2 -fpic  unlzss32.cpp

../../../../../root/work/TSIPRec2/Release_Build/TSIPRec2.so: ../../../../../root/work/TSIPRec2/Release_Build/BaseFileCombiner.o ../../../../../root/work/TSIPRec2/Release_Build/BitArrayObject.o \
         ../../../../../root/work/TSIPRec2/Release_Build/DecoderThread.o ../../../../../root/work/TSIPRec2/Release_Build/DirectroyHelp.o \
         ../../../../../root/work/TSIPRec2/Release_Build/DVBFileReceiver.o ../../../../../root/work/TSIPRec2/Release_Build/FileObject.o \
         ../../../../../root/work/TSIPRec2/Release_Build/HugeFile.o ../../../../../root/work/TSIPRec2/Release_Build/IPFileMendHelper.o \
         ../../../../../root/work/TSIPRec2/Release_Build/IPRDrvAPI.o ../../../../../root/work/TSIPRec2/Release_Build/IPRecSvr_i.o \
         ../../../../../root/work/TSIPRec2/Release_Build/Lzhuf.o ../../../../../root/work/TSIPRec2/Release_Build/MB_OneFile.o \
         ../../../../../root/work/TSIPRec2/Release_Build/StdAfx.o ../../../../../root/work/TSIPRec2/Release_Build/TSDB_Rec.o \
         ../../../../../root/work/TSIPRec2/Release_Build/TSDBFileSystem.o ../../../../../root/work/TSIPRec2/Release_Build/UDPDataPortLinux.o \
         ../../../../../root/work/TSIPRec2/Release_Build/UDPRecThread.o ../../../../../root/work/TSIPRec2/Release_Build/UnCmpMgr.o \
         ../../../../../root/work/TSIPRec2/Release_Build/UnCompressObj.o ../../../../../root/work/TSIPRec2/Release_Build/unlzss32.o \
        
	g++ -shared -o $(BUILD_DIR)/TSIPRec2.so  ../../../../../root/work/TSIPRec2/Release_Build/BaseFileCombiner.o ../../../../../root/work/TSIPRec2/Release_Build/BitArrayObject.o ../../../../../root/work/TSIPRec2/Release_Build/DecoderThread.o ../../../../../root/work/TSIPRec2/Release_Build/DirectroyHelp.o ../../../../../root/work/TSIPRec2/Release_Build/DVBFileReceiver.o ../../../../../root/work/TSIPRec2/Release_Build/FileObject.o ../../../../../root/work/TSIPRec2/Release_Build/HugeFile.o ../../../../../root/work/TSIPRec2/Release_Build/IPFileMendHelper.o ../../../../../root/work/TSIPRec2/Release_Build/IPRDrvAPI.o ../../../../../root/work/TSIPRec2/Release_Build/IPRecSvr_i.o ../../../../../root/work/TSIPRec2/Release_Build/Lzhuf.o ../../../../../root/work/TSIPRec2/Release_Build/MB_OneFile.o ../../../../../root/work/TSIPRec2/Release_Build/StdAfx.o ../../../../../root/work/TSIPRec2/Release_Build/TSDB_Rec.o ../../../../../root/work/TSIPRec2/Release_Build/TSDBFileSystem.o ../../../../../root/work/TSIPRec2/Release_Build/UDPDataPortLinux.o ../../../../../root/work/TSIPRec2/Release_Build/UDPRecThread.o ../../../../../root/work/TSIPRec2/Release_Build/UnCmpMgr.o ../../../../../root/work/TSIPRec2/Release_Build/UnCompressObj.o ../../../../../root/work/TSIPRec2/Release_Build/unlzss32.o

builddir:
	-mkdir -p ..
	-mkdir -p ../root
	-mkdir -p ../root/work
	-mkdir -p ../root/work/TSIPRec2
	-mkdir -p ../root/work/TSIPRec2/Release_Build

clean:
	-rm ../../../../../root/work/TSIPRec2/Release_Build/BaseFileCombiner.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/BaseFileCombiner.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/BitArrayObject.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/BitArrayObject.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/DecoderThread.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/DecoderThread.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/DirectroyHelp.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/DirectroyHelp.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/DVBFileReceiver.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/DVBFileReceiver.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/FileObject.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/FileObject.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/HugeFile.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/HugeFile.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/IPFileMendHelper.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/IPFileMendHelper.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/IPRDrvAPI.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/IPRDrvAPI.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/IPRecSvr_i.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/IPRecSvr_i.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/Lzhuf.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/Lzhuf.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/MB_OneFile.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/MB_OneFile.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/StdAfx.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/StdAfx.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/TSDB_Rec.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/TSDB_Rec.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/TSDBFileSystem.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/TSDBFileSystem.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UDPDataPortLinux.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UDPDataPortLinux.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UDPRecThread.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UDPRecThread.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UnCmpMgr.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UnCmpMgr.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UnCompressObj.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/UnCompressObj.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/unlzss32.o
	-rm ../../../../../root/work/TSIPRec2/Release_Build/unlzss32.d
	-rm ../../../../../root/work/TSIPRec2/Release_Build/TSIPRec2.so

endif
