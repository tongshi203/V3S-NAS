# Microsoft Developer Studio Project File - Name="BScript" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=BScript - Win32 StaticDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BScript.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BScript.mak" CFG="BScript - Win32 StaticDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BScript - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "BScript - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "BScript - Win32 StaticDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "BScript - Win32 StaticRelease" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BScript - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "E:\work\bscript\Release"
# PROP Intermediate_Dir "E:\work\bscript\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"F:\CYJ\MyCommonClass\ForVC\lib\BScript.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=DoUpdate.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "BScript - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "E:\work\bscript\Debug"
# PROP Intermediate_Dir "E:\work\bscript\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"F:\CYJ\MyCommonClass\ForVC\lib\BScriptD.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=DoUpdate.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticDebug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BScript___Win32_StaticDebug"
# PROP BASE Intermediate_Dir "BScript___Win32_StaticDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "E:\work\bscript\StaticDebug"
# PROP Intermediate_Dir "E:\work\bscript\StaticDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"F:\CYJ\MyCommonClass\ForVC\lib\BScriptD.lib"
# ADD LIB32 /nologo /out:"F:\CYJ\MyCommonClass\ForVC\lib\BScriptSD.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=DoUpdate.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticRelease"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BScript___Win32_StaticRelease"
# PROP BASE Intermediate_Dir "BScript___Win32_StaticRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "E:\work\bscript\StaticRelease"
# PROP Intermediate_Dir "E:\work\bscript\StaticRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"F:\CYJ\MyCommonClass\ForVC\lib\BScript.lib"
# ADD LIB32 /nologo /out:"F:\CYJ\MyCommonClass\ForVC\lib\BScriptS.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=DoUpdate.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "BScript - Win32 Release"
# Name "BScript - Win32 Debug"
# Name "BScript - Win32 StaticDebug"
# Name "BScript - Win32 StaticRelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\blib.c

!IF  "$(CFG)" == "BScript - Win32 Release"

!ELSEIF  "$(CFG)" == "BScript - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticDebug"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BScriptEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\context.c

!IF  "$(CFG)" == "BScript - Win32 Release"

!ELSEIF  "$(CFG)" == "BScript - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticDebug"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gram.c

!IF  "$(CFG)" == "BScript - Win32 Release"

!ELSEIF  "$(CFG)" == "BScript - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticDebug"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lex.c

!IF  "$(CFG)" == "BScript - Win32 Release"

!ELSEIF  "$(CFG)" == "BScript - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticDebug"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\module.c

!IF  "$(CFG)" == "BScript - Win32 Release"

!ELSEIF  "$(CFG)" == "BScript - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticDebug"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticRelease"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\runtime.c

!IF  "$(CFG)" == "BScript - Win32 Release"

!ELSEIF  "$(CFG)" == "BScript - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticDebug"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "BScript - Win32 StaticRelease"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\blib.h
# End Source File
# Begin Source File

SOURCE=.\bscript.h
# End Source File
# Begin Source File

SOURCE=.\BScriptEngine.h
# End Source File
# Begin Source File

SOURCE=.\gram.h
# End Source File
# Begin Source File

SOURCE=.\intern.h
# End Source File
# End Group
# End Target
# End Project
