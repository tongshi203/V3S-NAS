# Microsoft Developer Studio Project File - Name="aacenclib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=aacenclib - Win32 Mono Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "aacenclib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "aacenclib.mak" CFG="aacenclib - Win32 Mono Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aacenclib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aacenclib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "aacenclib - Win32 Mono Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aacenclib - Win32 Mono Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "aacenclib - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_resamplib" /I "../etsiop_sbrenc" /I "../etsiop_sbrenc/src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "aacenclib - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_resamplib" /I "../etsiop_sbrenc" /I "../etsiop_sbrenc/src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /GZ /c
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "aacenclib - Win32 Mono Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib\Win32_O"
# PROP Intermediate_Dir ".\lib\Win32_O\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_resamplib" /I "../etsiop_sbrenc" /I "../etsiop_sbrenc/src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MONO_ONLY" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "aacenclib - Win32 Mono Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib\Win32_D"
# PROP Intermediate_Dir ".\lib\Win32_D\obj"
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "../src" /I "../etsiop_ffrlib" /I "../etsioplib" /I "../etsiop_bitbuf" /I "../etsiop_resamplib" /I "../etsiop_sbrenc" /I "../etsiop_sbrenc/src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MONO_ONLY" /YX /FD /GZ /c
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "aacenclib - Win32 Release"
# Name "aacenclib - Win32 Debug"
# Name "aacenclib - Win32 Mono Release"
# Name "aacenclib - Win32 Mono Debug"
# Begin Source File

SOURCE=.\src\aac_ram.c
# End Source File
# Begin Source File

SOURCE=.\src\aac_rom.c
# End Source File
# Begin Source File

SOURCE=.\src\aacenc.c
# End Source File
# Begin Source File

SOURCE=.\src\adj_thr.c
# End Source File
# Begin Source File

SOURCE=.\src\band_nrg.c
# End Source File
# Begin Source File

SOURCE=.\src\bit_cnt.c
# End Source File
# Begin Source File

SOURCE=.\src\bitenc.c
# End Source File
# Begin Source File

SOURCE=.\src\block_switch.c
# End Source File
# Begin Source File

SOURCE=.\src\channel_map.c
# End Source File
# Begin Source File

SOURCE=.\src\dyn_bits.c
# End Source File
# Begin Source File

SOURCE=.\src\fft.c
# End Source File
# Begin Source File

SOURCE=.\src\grp_data.c
# End Source File
# Begin Source File

SOURCE=.\src\interface.c
# End Source File
# Begin Source File

SOURCE=.\src\line_pe.c
# End Source File
# Begin Source File

SOURCE=.\src\ms_stereo.c
# End Source File
# Begin Source File

SOURCE=.\src\pre_echo_control.c
# End Source File
# Begin Source File

SOURCE=.\src\psy_configuration.c
# End Source File
# Begin Source File

SOURCE=.\src\psy_main.c
# End Source File
# Begin Source File

SOURCE=.\src\qc_main.c
# End Source File
# Begin Source File

SOURCE=.\src\quantize.c
# End Source File
# Begin Source File

SOURCE=.\src\sf_estim.c
# End Source File
# Begin Source File

SOURCE=.\src\spreading.c
# End Source File
# Begin Source File

SOURCE=.\src\stat_bits.c
# End Source File
# Begin Source File

SOURCE=.\src\stprepro.c
# End Source File
# Begin Source File

SOURCE=.\src\tns.c
# End Source File
# Begin Source File

SOURCE=.\src\tns_param.c
# End Source File
# Begin Source File

SOURCE=.\src\transform.c
# End Source File
# End Target
# End Project
