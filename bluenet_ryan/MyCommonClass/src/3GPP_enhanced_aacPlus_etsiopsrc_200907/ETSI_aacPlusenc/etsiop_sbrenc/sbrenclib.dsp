# Microsoft Developer Studio Project File - Name="sbrenclib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sbrenclib - Win32 Mono Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sbrenclib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sbrenclib.mak" CFG="sbrenclib - Win32 Mono Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sbrenclib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sbrenclib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sbrenclib - Win32 Mono Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sbrenclib - Win32 Mono Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "sbrenclib - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_fastaacenc" /I "../etsiop_fastaacenc/src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_fastaacenc" /I "../etsiop_fastaacenc/src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /GZ /c
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_fastaacenc" /I "../etsiop_fastaacenc/src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MONO_ONLY" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_fastaacenc" /I "../etsiop_fastaacenc/src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MONO_ONLY" /YX /FD /GZ /c
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "sbrenclib - Win32 Release"
# Name "sbrenclib - Win32 Debug"
# Name "sbrenclib - Win32 Mono Release"
# Name "sbrenclib - Win32 Mono Debug"
# Begin Source File

SOURCE=.\src\bit_sbr.c
# End Source File
# Begin Source File

SOURCE=.\src\code_env.c
# End Source File
# Begin Source File

SOURCE=.\src\env_bit.c
# End Source File
# Begin Source File

SOURCE=.\src\env_est.c
# End Source File
# Begin Source File

SOURCE=.\src\fram_gen.c
# End Source File
# Begin Source File

SOURCE=.\src\freq_sca.c
# End Source File
# Begin Source File

SOURCE=.\src\hybrid.c

!IF  "$(CFG)" == "sbrenclib - Win32 Release"

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Debug"

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\invf_est.c
# End Source File
# Begin Source File

SOURCE=.\src\mh_det.c
# End Source File
# Begin Source File

SOURCE=.\src\nf_est.c
# End Source File
# Begin Source File

SOURCE=.\src\ps_bitenc.c

!IF  "$(CFG)" == "sbrenclib - Win32 Release"

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Debug"

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\ps_enc.c

!IF  "$(CFG)" == "sbrenclib - Win32 Release"

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Debug"

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sbrenclib - Win32 Mono Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\qmf_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\sbr_main.c
# End Source File
# Begin Source File

SOURCE=.\src\sbr_misc.c
# End Source File
# Begin Source File

SOURCE=.\src\sbr_ram.c
# End Source File
# Begin Source File

SOURCE=.\src\sbr_rom.c
# End Source File
# Begin Source File

SOURCE=.\src\ton_corr.c
# End Source File
# Begin Source File

SOURCE=.\src\tran_det.c
# End Source File
# End Target
# End Project
