# Microsoft Developer Studio Project File - Name="tst2rts" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=tst2rts - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tst2rts.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tst2rts.mak" CFG="tst2rts - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tst2rts - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tst2rts - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tst2rts - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D for="if (0) ; else for" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng1.lib jpegwrap.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "tst2rts - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D for="if (0) ; else for" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng1.lib jpegwrap.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "tst2rts - Win32 Release"
# Name "tst2rts - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\common\common_palettes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\DefaultFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\DefaultFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Filters.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Image32.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Tile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Tileset.cpp
# End Source File
# Begin Source File

SOURCE=.\tst2rts.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\common\begin_packed_struct.h
# End Source File
# Begin Source File

SOURCE=..\..\common\common_palettes.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\DefaultFile.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\DefaultFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\end_packed_struct.h
# End Source File
# Begin Source File

SOURCE=..\..\common\Filters.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IFile.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Image32.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Interface.hpp
# End Source File
# Begin Source File

SOURCE=..\..\jpegwrap\jpegwrap.h
# End Source File
# Begin Source File

SOURCE=..\..\common\packed.hpp
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\png\png.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\png\pngconf.h"
# End Source File
# Begin Source File

SOURCE=..\..\common\primitives.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\rgb.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\strcmp_ci.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Tile.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Tileset.hpp
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\zlib\zconf.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\zlib\zlib.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
