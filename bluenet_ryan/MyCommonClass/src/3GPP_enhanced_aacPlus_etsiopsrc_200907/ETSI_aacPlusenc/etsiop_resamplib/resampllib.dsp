# Microsoft Developer Studio Project File - Name="resampllib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=resampllib - Win32 Mono Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "resampllib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "resampllib.mak" CFG="resampllib - Win32 Mono Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "resampllib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "resampllib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "resampllib - Win32 Mono Release" (based on "Win32 (x86) Static Library")
!MESSAGE "resampllib - Win32 Mono Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "resampllib - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "../etsiop_ffrlib" /I "../etsioplib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "resampllib - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "../etsiop_ffrlib" /I "../etsioplib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "resampllib - Win32 Mono Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "../etsiop_ffrlib" /I "../etsioplib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MONO_ONLY" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "resampllib - Win32 Mono Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "../etsiop_ffrlib" /I "../etsioplib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MONO_ONLY" /YX /FD /GZ /c
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "resampllib - Win32 Release"
# Name "resampllib - Win32 Debug"
# Name "resampllib - Win32 Mono Release"
# Name "resampllib - Win32 Mono Debug"
# Begin Source File

SOURCE=.\src\downsample_FIR.c
# End Source File
# End Target
# End Project
