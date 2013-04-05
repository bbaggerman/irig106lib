# Microsoft Developer Studio Project File - Name="irig106" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=irig106 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "irig106.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "irig106.mak" CFG="irig106 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "irig106 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "irig106 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "irig106 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "static\release"
# PROP Intermediate_Dir "static\release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "irig106 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "static\debug"
# PROP Intermediate_Dir "static\debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "irig106 - Win32 Release"
# Name "irig106 - Win32 Debug"
# Begin Source File

SOURCE=..\src\config.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_data_stream.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_data_stream.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_1553f1.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_1553f1.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_arinc429.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_arinc429.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_discrete.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_discrete.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_ethernet.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_ethernet.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_time.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_time.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_tmats.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_tmats.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_uart.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_uart.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_video.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_decode_video.h
# End Source File
# Begin Source File

SOURCE=..\src\i106_time.c
# End Source File
# Begin Source File

SOURCE=..\src\i106_time.h
# End Source File
# Begin Source File

SOURCE=..\src\Irig106Ch10.c
# End Source File
# Begin Source File

SOURCE=..\src\Irig106Ch10.h
# End Source File
# Begin Source File

SOURCE=..\src\irig106cl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\irig106cl.h
# End Source File
# Begin Source File

SOURCE=..\src\stdint.h
# End Source File
# End Target
# End Project
