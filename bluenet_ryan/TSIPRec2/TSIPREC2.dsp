# Microsoft Developer Studio Project File - Name="TSIPREC2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TSIPREC2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSIPREC2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSIPREC2.mak" CFG="TSIPREC2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSIPREC2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TSIPREC2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/DVB/IP文件接收驱动_CPP_接口", BWFAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSIPREC2 - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "E:\work\tsiprec2\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /D FD_SETSIZE=128 /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 Ws2_32.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "TSIPREC2 - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "E:\work\tsiprec2\"
# PROP Intermediate_Dir "E:\work\tsiprec2\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /D FD_SETSIZE=128 /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Ws2_32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "TSIPREC2 - Win32 Release"
# Name "TSIPREC2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BaseFileCombiner.cpp
# End Source File
# Begin Source File

SOURCE=.\BitArrayObject.cpp
# End Source File
# Begin Source File

SOURCE=.\DecoderThread.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectroyHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\DVBEPGReceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\DVBFileReceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\FileDelayEventDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\FileObject.cpp
# End Source File
# Begin Source File

SOURCE=.\FileUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\FileWriterThread.cpp
# End Source File
# Begin Source File

SOURCE=.\HugeFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IPEncryptKeyMgrImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\IPFileMendHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\IPRDrvAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\IPUnlockDrvMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\IPUnlockDrvWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\LicDecode.cpp
# End Source File
# Begin Source File

SOURCE=.\Lzhuf.cpp
# End Source File
# Begin Source File

SOURCE=.\MB_OneFile.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TSDB_Rec.cpp
# End Source File
# Begin Source File

SOURCE=.\TSDBFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\TSDBHugeFileObj.cpp
# End Source File
# Begin Source File

SOURCE=.\TSIPREC2.cpp
# End Source File
# Begin Source File

SOURCE=.\TSIPREC2.def
# End Source File
# Begin Source File

SOURCE=.\TSIPREC2.rc
# End Source File
# Begin Source File

SOURCE=.\UDPDataPortLinux.cpp
# End Source File
# Begin Source File

SOURCE=.\UDPRecThread.cpp
# End Source File
# Begin Source File

SOURCE=.\UnCmpMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\UnCompressObj.cpp
# End Source File
# Begin Source File

SOURCE=.\unlzss32.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BaseFileCombiner.h
# End Source File
# Begin Source File

SOURCE=.\BitArrayObject.h
# End Source File
# Begin Source File

SOURCE=.\CodecFormat.h
# End Source File
# Begin Source File

SOURCE=.\CODUNLOCKDRVAPI.H
# End Source File
# Begin Source File

SOURCE=.\DecoderThread.h
# End Source File
# Begin Source File

SOURCE=.\DirectroyHelp.h
# End Source File
# Begin Source File

SOURCE=.\DPRecThread.h
# End Source File
# Begin Source File

SOURCE=.\DVBEPGReceiver.h
# End Source File
# Begin Source File

SOURCE=.\DVBFileReceiver.h
# End Source File
# Begin Source File

SOURCE=.\FileDelayEventDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\FileObject.h
# End Source File
# Begin Source File

SOURCE=.\FilePurpose.h
# End Source File
# Begin Source File

SOURCE=.\FileUpdate.h
# End Source File
# Begin Source File

SOURCE=.\FileWriterThread.h
# End Source File
# Begin Source File

SOURCE=.\GetBindIP.h
# End Source File
# Begin Source File

SOURCE=.\HugeFile.h
# End Source File
# Begin Source File

SOURCE=.\IPData.h
# End Source File
# Begin Source File

SOURCE=.\IPEncryptDataStruct.h
# End Source File
# Begin Source File

SOURCE=.\IPEncryptKeyMgrImpl.h
# End Source File
# Begin Source File

SOURCE=.\IPFileMendHelper.h
# End Source File
# Begin Source File

SOURCE=.\IPRecSvr.h
# End Source File
# Begin Source File

SOURCE=.\IPUnlockDrvMgr.h
# End Source File
# Begin Source File

SOURCE=.\IPUnlockDrvWrapper.h
# End Source File
# Begin Source File

SOURCE=.\LicDecode.h
# End Source File
# Begin Source File

SOURCE=.\Lzhuf.h
# End Source File
# Begin Source File

SOURCE=.\MB_OneFile.h
# End Source File
# Begin Source File

SOURCE=.\MyGetVariantOptionalVal.h
# End Source File
# Begin Source File

SOURCE=.\MyIUnknownImpl.h
# End Source File
# Begin Source File

SOURCE=.\MyRegKey.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SrcDataPort.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Tsdb.h
# End Source File
# Begin Source File

SOURCE=.\TSDB_Rec.h
# End Source File
# Begin Source File

SOURCE=.\TSDBFileSystem.h
# End Source File
# Begin Source File

SOURCE=.\TSDBHugeFileMgr.h
# End Source File
# Begin Source File

SOURCE=.\TSDBHugeFileObj.h
# End Source File
# Begin Source File

SOURCE=.\TSDVBBROPROTOCOL.H
# End Source File
# Begin Source File

SOURCE=.\TSIPREC2.h
# End Source File
# Begin Source File

SOURCE=.\UDPDataPortLinux.h
# End Source File
# Begin Source File

SOURCE=.\UDPRecThread.h
# End Source File
# Begin Source File

SOURCE=.\UnCmpMgr.h
# End Source File
# Begin Source File

SOURCE=.\UnCompressObj.h
# End Source File
# Begin Source File

SOURCE=.\unlzss32.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\TSIPREC2.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=".\版本升级记录.txt"
# End Source File
# End Target
# End Project
