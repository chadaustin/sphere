# Microsoft Developer Studio Project File - Name="sdl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sdl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sdl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sdl.mak" CFG="sdl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sdl - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sdl - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sdl - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "XP_PC" /D "SPHERE_SDL" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D for="if (0) ; else for" /D "ZLIB_DLL" /D "MNG_USE_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib js32.lib zlib.lib libpng1.lib jpegwrap.lib libmng.lib SDLmain.lib SDL.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "sdl - Win32 Debug"

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
# ADD CPP /nologo /G5 /MDd /W3 /GX /ZI /Od /D "_DEBUG" /D "XP_PC" /D "SPHERE_SDL" /D "WIN32" /D "WINDOWS" /D "_CONSOLE" /D "_MBCS" /D for="if (0) ; else for" /D "ZLIB_DLL" /D "JSFILE" /D "MNG_USE_DLL" /D "MNG_SKIP_LCMS" /D "MNG_SKIP_IJG6B" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib js32.lib libpng1.lib jpegwrap.lib zlib.lib libmng.lib ws2_32.lib SDLmain.lib SDL.lib HawkNL.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"msvcrt" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sdl - Win32 Release"
# Name "sdl - Win32 Debug"
# Begin Group "SDL Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\sdl_audio.cpp
# End Source File
# Begin Source File

SOURCE=.\sdl_audio.hpp
# End Source File
# Begin Source File

SOURCE=.\sdl_input.cpp
# End Source File
# Begin Source File

SOURCE=.\sdl_input.hpp
# End Source File
# Begin Source File

SOURCE=.\sdl_internal.hpp
# End Source File
# Begin Source File

SOURCE=.\sdl_main.cpp
# End Source File
# Begin Source File

SOURCE=.\sdl_network.cpp
# End Source File
# Begin Source File

SOURCE=.\sdl_network.hpp
# End Source File
# Begin Source File

SOURCE=.\sdl_screenshot.cpp
# End Source File
# Begin Source File

SOURCE=.\sdl_time.cpp
# End Source File
# Begin Source File

SOURCE=.\sdl_time.hpp
# End Source File
# Begin Source File

SOURCE=.\sdl_video.cpp
# End Source File
# Begin Source File

SOURCE=.\sdl_video.hpp
# End Source File
# Begin Source File

SOURCE=..\win32\win32_filesystem.cpp
# End Source File
# Begin Source File

SOURCE=..\win32\win32_filesystem.hpp
# End Source File
# End Group
# Begin Group "Library"

# PROP Default_Filter ""
# Begin Group "SDL"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL-1.0.8\include\begin_code.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL-1.0.8\include\close_code.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_active.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_audio.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_byteorder.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_cdrom.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_error.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_events.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_keyboard.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_keysym.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_main.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_mouse.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_mutex.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_quit.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_rwops.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_timer.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_types.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_version.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\SDL\include\SDL_video.h"
# End Source File
# End Group
# Begin Group "JavaScript"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jsapi.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jscompat.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jscpucfg.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jslong.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jsosdep.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jsotypes.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jspubtd.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\third-party\js32\jstypes.h"
# End Source File
# End Group
# Begin Group "PNG"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\..\Third-Party\png\png.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\Third-Party\png\pngconf.h"
# End Source File
# End Group
# Begin Group "MNG"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\..\Third-Party\libmng\libmng.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\Third-Party\libmng\libmng_conf.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\Third-Party\libmng\libmng_types.h"
# End Source File
# End Group
# Begin Group "Jpeg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\jpegwrap\jpegwrap.h
# End Source File
# End Group
# Begin Source File

SOURCE="..\..\..\..\Third-Party\libmng\contrib\vc6\lcms\include\lcms.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\Third-Party\zlib\zconf.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\Third-Party\zlib\zlib.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\Third-Party\png\msvc\win32\zlib\dll\zlib.lib"
# End Source File
# End Group
# Begin Group "Sphere"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\benchmark.cpp
# End Source File
# Begin Source File

SOURCE=..\benchmark.hpp
# End Source File
# Begin Source File

SOURCE=..\engine.cpp
# End Source File
# Begin Source File

SOURCE=..\engine.hpp
# End Source File
# Begin Source File

SOURCE=..\engineinterface.hpp
# End Source File
# Begin Source File

SOURCE=..\inputx.hpp
# End Source File
# Begin Source File

SOURCE=..\log.cpp
# End Source File
# Begin Source File

SOURCE=..\map_engine.cpp
# End Source File
# Begin Source File

SOURCE=..\map_engine.hpp
# End Source File
# Begin Source File

SOURCE=..\menu.cpp
# End Source File
# Begin Source File

SOURCE=..\menu.hpp
# End Source File
# Begin Source File

SOURCE=..\render.cpp
# End Source File
# Begin Source File

SOURCE=..\render.hpp
# End Source File
# Begin Source File

SOURCE=..\rendersort.cpp
# End Source File
# Begin Source File

SOURCE=..\rendersort.hpp
# End Source File
# Begin Source File

SOURCE=..\script.cpp
# End Source File
# Begin Source File

SOURCE=..\script.hpp
# End Source File
# Begin Source File

SOURCE=..\sfont.cpp
# End Source File
# Begin Source File

SOURCE=..\sfont.hpp
# End Source File
# Begin Source File

SOURCE=..\simage.cpp
# End Source File
# Begin Source File

SOURCE=..\simage.hpp
# End Source File
# Begin Source File

SOURCE=..\smap.cpp
# End Source File
# Begin Source File

SOURCE=..\smap.hpp
# End Source File
# Begin Source File

SOURCE=..\sphere.cpp
# End Source File
# Begin Source File

SOURCE=..\sphere.hpp
# End Source File
# Begin Source File

SOURCE=..\SpritesetServer.cpp
# End Source File
# Begin Source File

SOURCE=..\ss_functions.table
# End Source File
# Begin Source File

SOURCE=..\sspriteset.cpp
# End Source File
# Begin Source File

SOURCE=..\sspriteset.hpp
# End Source File
# Begin Source File

SOURCE=..\swindowstyle.cpp
# End Source File
# Begin Source File

SOURCE=..\swindowstyle.hpp
# End Source File
# End Group
# Begin Group "Sphere Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\common\AnimationFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\AnimationFactory.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\begin_packed_struct.h
# End Source File
# Begin Source File

SOURCE=..\..\common\common_palettes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\configfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\configfile.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\DefaultFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\DefaultFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\DefaultFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\end_packed_struct.h
# End Source File
# Begin Source File

SOURCE=..\..\common\Entities.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Entities.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Filters.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\FLICAnimation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\FLICAnimation.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Font.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Font.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IFile.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Image32.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Image32.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Layer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Layer.hpp
# End Source File
# Begin Source File

SOURCE=..\log.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Map.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Map.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\MNGAnimation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\MNGAnimation.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\ObstructionMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\ObstructionMap.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\PackageFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\PackageFile.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\PackageFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\PackageFileSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\packed.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\primitives.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\rgb.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\spk.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Sprite.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Spriteset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Spriteset.hpp
# End Source File
# Begin Source File

SOURCE=..\SpritesetServer.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\strcmp_ci.hpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Tile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\Tileset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\WindowStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\WindowStyle.hpp
# End Source File
# End Group
# Begin Group "Platform Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\filesystem.hpp
# End Source File
# Begin Source File

SOURCE=..\input.hpp
# End Source File
# Begin Source File

SOURCE=..\network.hpp
# End Source File
# Begin Source File

SOURCE=..\timer.hpp
# End Source File
# Begin Source File

SOURCE=..\video.hpp
# End Source File
# End Group
# End Target
# End Project
