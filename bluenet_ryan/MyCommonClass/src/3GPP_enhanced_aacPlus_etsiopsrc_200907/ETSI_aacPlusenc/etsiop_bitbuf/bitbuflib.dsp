# Microsoft Developer Studio Project File - Name="bitbuflib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bitbuflib - Win32 Mono Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bitbuflib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bitbuflib.mak" CFG="bitbuflib - Win32 Mono Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bitbuflib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bitbuflib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bitbuflib - Win32 Mono Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bitbuflib - Win32 Mono Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "bitbuflib - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "../etsioplib" /I "../etsiop_ffrlib" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bitbuflib - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../etsioplib" /I "../etsiop_ffrlib" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bitbuflib - Win32 Mono Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../etsioplib" /I "../etsiop_ffrlib" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "MONO_ONLY" /D "LP_SBR_ONLY" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bitbuflib - Win32 Mono Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "../etsioplib" /I "../etsiop_ffrlib" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "MONO_ONLY" /D "LP_SBR_ONLY" /YX /FD /c
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "bitbuflib - Win32 Release"
# Name "bitbuflib - Win32 Debug"
# Name "bitbuflib - Win32 Mono Debug"
# Name "bitbuflib - Win32 Mono Release"
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\debug_lvl.h
# End Source File
# Begin Source File

SOURCE=.\FFR_bitbuffer.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\bitbuffer.c
# End Source File
# End Group
# End Target
# End Project
