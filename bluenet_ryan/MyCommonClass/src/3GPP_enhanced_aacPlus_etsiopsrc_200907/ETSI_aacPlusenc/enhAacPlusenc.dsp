# Microsoft Developer Studio Project File - Name="enhAacPlusenc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=enhAacPlusenc - Win32 Mono Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "enhAacPlusenc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "enhAacPlusenc.mak" CFG="enhAacPlusenc - Win32 Mono Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "enhAacPlusenc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "enhAacPlusenc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "enhAacPlusenc - Win32 Mono Release" (based on "Win32 (x86) Console Application")
!MESSAGE "enhAacPlusenc - Win32 Mono Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "enhAacPlusenc - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Win32_O"
# PROP Intermediate_Dir ".\Win32_O\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "./3g_lib" /I "./3g_lib/w32" /I "./etsiop_ffrlib" /I "./etsioplib" /I "./etsiop_bitbuf" /I "./etsiop_fastaacenc" /I "./etsiop_resamplib" /I "./etsiop_sbrenc" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy        .\3g_lib\w32\ct-libisomedia.dll        .\Win32_O\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "enhAacPlusenc - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Win32_D"
# PROP Intermediate_Dir ".\Win32_D\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "./3g_lib" /I "./3g_lib/w32" /I "./etsiop_ffrlib" /I "./etsioplib" /I "./etsiop_bitbuf" /I "./etsiop_fastaacenc" /I "./etsiop_resamplib" /I "./etsiop_sbrenc" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy        .\3g_lib\w32\ct-libisomedia.dll        .\Win32_D\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "enhAacPlusenc - Win32 Mono Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Win32_O"
# PROP Intermediate_Dir ".\Win32_O\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "./3g_lib" /I "./3g_lib/w32" /I "./etsiop_ffrlib" /I "./etsioplib" /I "./etsiop_bitbuf" /I "./etsiop_fastaacenc" /I "./etsiop_resamplib" /I "./etsiop_sbrenc" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "MONO_ONLY" /YX /FD /c
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:".\Win32_O/enhAacPlusEnc_mono.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy        .\3g_lib\w32\ct-libisomedia.dll        .\Win32_O\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "enhAacPlusenc - Win32 Mono Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Win32_D"
# PROP Intermediate_Dir ".\Win32_D\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "./3g_lib" /I "./3g_lib/w32" /I "./etsiop_ffrlib" /I "./etsioplib" /I "./etsiop_bitbuf" /I "./etsiop_fastaacenc" /I "./etsiop_resamplib" /I "./etsiop_sbrenc" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "MONO_ONLY" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:".\Win32_D/enhAacPlusEnc_mono.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy        .\3g_lib\w32\ct-libisomedia.dll        .\Win32_D\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "enhAacPlusenc - Win32 Release"
# Name "enhAacPlusenc - Win32 Debug"
# Name "enhAacPlusenc - Win32 Mono Release"
# Name "enhAacPlusenc - Win32 Mono Debug"
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=.\src\mp4file.c
# End Source File
# Begin Source File

SOURCE=".\3g_lib\w32\ct-libisomedia.lib"
# End Source File
# Begin Source File

SOURCE=.\3g_lib\w32\audiolib.lib
# End Source File
# End Target
# End Project
