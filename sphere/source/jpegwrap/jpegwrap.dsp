# Microsoft Developer Studio Project File - Name="jpegwrap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=jpegwrap - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jpegwrap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jpegwrap.mak" CFG="jpegwrap - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jpegwrap - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jpegwrap - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jpegwrap - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JPEGWRAP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JPEGWRAP_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "jpegwrap - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JPEGWRAP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JPEGWRAP_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "jpegwrap - Win32 Release"
# Name "jpegwrap - Win32 Debug"
# Begin Group "Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\jpegwrap.cpp
# End Source File
# Begin Source File

SOURCE=.\jpegwrap.def
# End Source File
# Begin Source File

SOURCE=.\jpegwrap.h
# End Source File
# End Group
# Begin Group "jpeglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcapimin.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcapistd.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jccoefct.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jccolor.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcdctmgr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jchuff.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jchuff.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcinit.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcmainct.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcmarker.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcmaster.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcomapi.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jconfig.cfg"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jconfig.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcparam.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcphuff.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcprepct.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jcsample.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jctrans.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdapimin.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdapistd.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdatadst.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdatasrc.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdcoefct.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdcolor.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdct.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jddctmgr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdhuff.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdhuff.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdinput.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdmainct.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdmarker.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdmaster.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdmerge.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdphuff.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdpostct.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdsample.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jdtrans.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jerror.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jerror.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jfdctflt.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jfdctfst.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jfdctint.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jidctflt.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jidctfst.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jidctint.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jidctred.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jinclude.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jmemmgr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jmemnobs.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jmemsys.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jmorecfg.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jpegint.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jpeglib.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jquant1.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jquant2.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jutils.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\jversion.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\makefile.cfg"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\transupp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\third-party\jpeg\transupp.h"
# End Source File
# End Group
# End Target
# End Project
