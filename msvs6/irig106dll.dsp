# Microsoft Developer Studio Project File - Name="irig106dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=irig106dll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "irig106dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "irig106dll.mak" CFG="irig106dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "irig106dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "irig106dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "irig106dll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "dll\release"
# PROP Intermediate_Dir "dll\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRIG106DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"dll\release\irig106.dll"

!ELSEIF  "$(CFG)" == "irig106dll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "dll\debug"
# PROP Intermediate_Dir "dll\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRIG106DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"dll\debug\irig106.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "irig106dll - Win32 Release"
# Name "irig106dll - Win32 Debug"
# Begin Source File

SOURCE=..\src\config.h
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

SOURCE=..\src\irig106ch10.c
# End Source File
# Begin Source File

SOURCE=..\src\Irig106Ch10.h
# End Source File
# Begin Source File

SOURCE=..\src\irig106dll.def
# End Source File
# Begin Source File

SOURCE=..\src\stdint.h
# End Source File
# End Target
# End Project
