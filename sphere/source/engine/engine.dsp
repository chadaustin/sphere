# Microsoft Developer Studio Project File - Name="engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=engine - Win32 Profile
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak" CFG="engine - Win32 Profile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "engine - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "output/Release"
# PROP Intermediate_Dir "output/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GX /O1 /I "../../third-party-msvc/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "STRICT" /D "XP_PC" /D "JSFILE" /D for="if (0) ; else for" /D "ZLIB_DLL" /D "MNG_USE_DLL" /D "MNG_SKIP_LCMS" /D "MNG_SKIP_IJG6B" /D "NOMINMAX" /D "USE_SPRITESET_SERVER" /FAs /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib corona.lib audiere.lib winmm.lib js32.lib zlib.lib libmng.lib /nologo /subsystem:windows /pdb:none /machine:I386 /libpath:"../../third-party-msvc/lib" /opt:nowin98
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=install_engine.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "output/Debug"
# PROP Intermediate_Dir "output/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /GX /ZI /Od /I "../../third-party-msvc/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "STRICT" /D "XP_PC" /D "JSFILE" /D for="if (0) ; else for" /D "ZLIB_DLL" /D "MNG_USE_DLL" /D "MNG_SKIP_LCMS" /D "MNG_SKIP_IJG6B" /D "NOMINMAX" /FAs /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib corona.lib audiere.lib winmm.lib js32.lib zlib.lib libmng.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../third-party-msvc/lib"
# SUBTRACT LINK32 /pdb:none /incremental:no /map /nodefaultlib

!ELSEIF  "$(CFG)" == "engine - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "engine___Win32_Profile"
# PROP BASE Intermediate_Dir "engine___Win32_Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "output/Profile"
# PROP Intermediate_Dir "output/Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /Zp1 /MDd /W3 /GX /ZI /Od /I "../../../js" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "STRICT" /D "XP_PC" /D "JSFILE" /FAs /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /GX /ZI /Od /I "../../third-party-msvc/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "STRICT" /D "XP_PC" /D "JSFILE" /D for="if (0) ; else for" /D "ZLIB_DLL" /D "MNG_USE_DLL" /D "MNG_SKIP_LCMS" /D "MNG_SKIP_IJG6B" /D "NOMINMAX" /D "USE_SPRITESET_SERVER" /FAs /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib corona.lib audiere.lib winmm.lib js32.lib zlib.lib libmng.lib /nologo /subsystem:windows /profile /debug /machine:I386 /libpath:"../../third-party-msvc/lib"

!ENDIF 

# Begin Target

# Name "engine - Win32 Release"
# Name "engine - Win32 Debug"
# Name "engine - Win32 Profile"
# Begin Group "Engine"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\benchmark.cpp
# End Source File
# Begin Source File

SOURCE=.\benchmark.hpp
# End Source File
# Begin Source File

SOURCE=.\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\engine.hpp
# End Source File
# Begin Source File

SOURCE=.\engineinterface.hpp
# End Source File
# Begin Source File

SOURCE=.\inputx.hpp
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# Begin Source File

SOURCE=.\log.hpp
# End Source File
# Begin Source File

SOURCE=.\map_engine.cpp
# End Source File
# Begin Source File

SOURCE=.\map_engine.hpp
# End Source File
# Begin Source File

SOURCE=.\render.cpp
# End Source File
# Begin Source File

SOURCE=.\render.hpp
# End Source File
# Begin Source File

SOURCE=.\rendersort.cpp
# End Source File
# Begin Source File

SOURCE=.\rendersort.hpp
# End Source File
# Begin Source File

SOURCE=.\script.cpp
# End Source File
# Begin Source File

SOURCE=.\script.hpp
# End Source File
# Begin Source File

SOURCE=.\sfont.cpp
# End Source File
# Begin Source File

SOURCE=.\sfont.hpp
# End Source File
# Begin Source File

SOURCE=.\smap.cpp
# End Source File
# Begin Source File

SOURCE=.\smap.hpp
# End Source File
# Begin Source File

SOURCE=.\sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\sphere.hpp
# End Source File
# Begin Source File

SOURCE=.\spritesetserver.cpp
# End Source File
# Begin Source File

SOURCE=.\spritesetserver.hpp
# End Source File
# Begin Source File

SOURCE=.\ss_functions.table
# End Source File
# Begin Source File

SOURCE=.\sspriteset.cpp
# End Source File
# Begin Source File

SOURCE=.\sspriteset.hpp
# End Source File
# Begin Source File

SOURCE=.\swindowstyle.cpp
# End Source File
# Begin Source File

SOURCE=.\swindowstyle.hpp
# End Source File
# Begin Source File

SOURCE=.\system.hpp
# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32\engine.ico
# End Source File
# Begin Source File

SOURCE=.\win32\engine.rc
# End Source File
# Begin Source File

SOURCE=.\win32\win32_audio.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_audio.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_filesystem.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_filesystem.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_input.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_input.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_internal.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_main.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_network.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_network.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_screenshot.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_sphere_config.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_sphere_config.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_time.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_time.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_video.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_video.hpp
# End Source File
# Begin Source File

SOURCE=.\win32\win32_wmain.cpp
# End Source File
# End Group
# Begin Group "Sphere Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\AnimationFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\common\AnimationFactory.hpp
# End Source File
# Begin Source File

SOURCE=..\common\common_palettes.cpp
# End Source File
# Begin Source File

SOURCE=..\common\common_palettes.hpp
# End Source File
# Begin Source File

SOURCE=..\common\configfile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\configfile.hpp
# End Source File
# Begin Source File

SOURCE=..\common\DefaultFile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\DefaultFile.hpp
# End Source File
# Begin Source File

SOURCE=..\common\DefaultFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\common\DefaultFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Entities.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Entities.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Filters.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Filters.hpp
# End Source File
# Begin Source File

SOURCE=..\common\FLICAnimation.cpp
# End Source File
# Begin Source File

SOURCE=..\common\FLICAnimation.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Font.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Font.hpp
# End Source File
# Begin Source File

SOURCE=..\common\IAnimation.hpp
# End Source File
# Begin Source File

SOURCE=..\common\IFile.hpp
# End Source File
# Begin Source File

SOURCE=..\common\IFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Image32.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Image32.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Interface.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Layer.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Layer.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Map.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Map.hpp
# End Source File
# Begin Source File

SOURCE=..\common\md5.c
# End Source File
# Begin Source File

SOURCE=..\common\md5.h
# End Source File
# Begin Source File

SOURCE=..\common\md5global.h
# End Source File
# Begin Source File

SOURCE=..\common\MNGAnimation.cpp
# End Source File
# Begin Source File

SOURCE=..\common\MNGAnimation.hpp
# End Source File
# Begin Source File

SOURCE=..\common\ObstructionMap.cpp
# End Source File
# Begin Source File

SOURCE=..\common\ObstructionMap.hpp
# End Source File
# Begin Source File

SOURCE=..\common\PackageFile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PackageFile.hpp
# End Source File
# Begin Source File

SOURCE=..\common\PackageFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PackageFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\common\packed.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Playlist.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Playlist.hpp
# End Source File
# Begin Source File

SOURCE=..\common\primitives.hpp
# End Source File
# Begin Source File

SOURCE=..\common\resample.cpp
# End Source File
# Begin Source File

SOURCE=..\common\resample.hpp
# End Source File
# Begin Source File

SOURCE=..\common\rgb.hpp
# End Source File
# Begin Source File

SOURCE=..\common\sphere_version.h
# End Source File
# Begin Source File

SOURCE=..\common\spk.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Spriteset.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Spriteset.hpp
# End Source File
# Begin Source File

SOURCE=..\common\strcmp_ci.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Tile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Tile.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Tileset.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Tileset.hpp
# End Source File
# Begin Source File

SOURCE=..\common\types.h
# End Source File
# Begin Source File

SOURCE=..\common\WindowStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\common\WindowStyle.hpp
# End Source File
# End Group
# Begin Group "Platform Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\audio.hpp
# End Source File
# Begin Source File

SOURCE=.\filesystem.hpp
# End Source File
# Begin Source File

SOURCE=.\input.hpp
# End Source File
# Begin Source File

SOURCE=.\network.hpp
# End Source File
# Begin Source File

SOURCE=.\time.hpp
# End Source File
# Begin Source File

SOURCE=.\video.hpp
# End Source File
# End Group
# End Target
# End Project
