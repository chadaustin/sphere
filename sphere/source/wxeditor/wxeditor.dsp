# Microsoft Developer Studio Project File - Name="wxeditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wxeditor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wxeditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wxeditor.mak" CFG="wxeditor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wxeditor - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wxeditor - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wxeditor - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../third-party-msvc/include" /D "NDEBUG" /D for="if (0) ; else for" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D WINVER=0x0400 /D "XP_PC" /D "MNG_USE_DLL" /D "MNG_SKIP_LCMS" /D "MNG_SKIP_IJG6B" /D "__WXMSW__" /FD /c
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
# ADD LINK32 wxmsw.lib stc.lib comctl32.lib ws2_32.lib rpcrt4.lib audiere.lib corona.lib js32.lib libmng.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../third-party-msvc/lib"

!ELSEIF  "$(CFG)" == "wxeditor - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../third-party-msvc/include" /D "_DEBUG" /D for="if (0) ; else for" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D WINVER=0x0400 /D "XP_PC" /D "MNG_USE_DLL" /D "MNG_SKIP_LCMS" /D "MNG_SKIP_IJG6B" /D "__WXMSW__" /FD /GZ /c
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
# ADD LINK32 wxmswd.lib stcd.lib comctl32.lib ws2_32.lib rpcrt4.lib audiere.lib corona.lib js32.lib libmng.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../third-party-msvc/lib"

!ENDIF 

# Begin Target

# Name "wxeditor - Win32 Release"
# Name "wxeditor - Win32 Debug"
# Begin Group "Core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AlphaView.cpp
# End Source File
# Begin Source File

SOURCE=.\AlphaView.hpp
# End Source File
# Begin Source File

SOURCE=.\AnimationWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimationWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\Audio.hpp
# End Source File
# Begin Source File

SOURCE=.\ColorView.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorView.hpp
# End Source File
# Begin Source File

SOURCE=.\Configuration.cpp
# End Source File
# Begin Source File

SOURCE=.\Configuration.hpp
# End Source File
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\Debug.hpp
# End Source File
# Begin Source File

SOURCE=.\DIBSection.cpp
# End Source File
# Begin Source File

SOURCE=.\DIBSection.hpp
# End Source File
# Begin Source File

SOURCE=.\DocumentWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\DocumentWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\Editor.cpp
# End Source File
# Begin Source File

SOURCE=.\Editor.hpp
# End Source File
# Begin Source File

SOURCE=.\EntityPersonDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\EntityPersonDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\EntityTriggerDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\EntityTriggerDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\FileDialogs.hpp
# End Source File
# Begin Source File

SOURCE=.\FileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\FileSystem.hpp
# End Source File
# Begin Source File

SOURCE=.\FileTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTypes.hpp
# End Source File
# Begin Source File

SOURCE=.\FontGradientDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FontGradientDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\FontWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\FontWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\GameSettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSettingsDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\HScrollWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\HScrollWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\IDs.hpp
# End Source File
# Begin Source File

SOURCE=.\ImageToolPalette.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageToolPalette.hpp
# End Source File
# Begin Source File

SOURCE=.\ImageView.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageView.hpp
# End Source File
# Begin Source File

SOURCE=.\ImageWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\Keys.cpp
# End Source File
# Begin Source File

SOURCE=.\Keys.hpp
# End Source File
# Begin Source File

SOURCE=.\LayerPropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\LayerPropertiesDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\LayerView.cpp
# End Source File
# Begin Source File

SOURCE=.\LayerView.hpp
# End Source File
# Begin Source File

SOURCE=.\MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MainWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\MapPropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\MapPropertiesDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\MapToolPalette.cpp
# End Source File
# Begin Source File

SOURCE=.\MapToolPalette.hpp
# End Source File
# Begin Source File

SOURCE=.\MapView.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView.hpp
# End Source File
# Begin Source File

SOURCE=.\MapWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MapWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\NewMapDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\NewMapDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\NewProjectDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\NewProjectDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\NumberDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\Package.cpp
# End Source File
# Begin Source File

SOURCE=.\Package.hpp
# End Source File
# Begin Source File

SOURCE=.\PaletteView.cpp
# End Source File
# Begin Source File

SOURCE=.\PaletteView.hpp
# End Source File
# Begin Source File

SOURCE=.\PaletteWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\PaletteWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\Project.cpp
# End Source File
# Begin Source File

SOURCE=.\Project.hpp
# End Source File
# Begin Source File

SOURCE=.\ProjectWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\ResizeDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizeDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\RGBAView.cpp
# End Source File
# Begin Source File

SOURCE=.\RGBAView.hpp
# End Source File
# Begin Source File

SOURCE=.\SaveableDocumentWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveableDocumentWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\Scripting.cpp
# End Source File
# Begin Source File

SOURCE=.\Scripting.hpp
# End Source File
# Begin Source File

SOURCE=.\ScriptWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\ScrollWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\ScrollWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\SoundWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\SpriteBaseView.cpp
# End Source File
# Begin Source File

SOURCE=.\SpriteBaseView.hpp
# End Source File
# Begin Source File

SOURCE=.\SpritePropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SpritePropertiesDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\SpritesetImagesPalette.cpp
# End Source File
# Begin Source File

SOURCE=.\SpritesetImagesPalette.hpp
# End Source File
# Begin Source File

SOURCE=.\SpritesetView.cpp
# End Source File
# Begin Source File

SOURCE=.\SpritesetView.hpp
# End Source File
# Begin Source File

SOURCE=.\SpritesetWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\SpritesetWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\StringDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\SwatchPalette.cpp
# End Source File
# Begin Source File

SOURCE=.\SwatchPalette.hpp
# End Source File
# Begin Source File

SOURCE=.\SwatchServer.cpp
# End Source File
# Begin Source File

SOURCE=.\SwatchServer.hpp
# End Source File
# Begin Source File

SOURCE=.\TileObstructionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TileObstructionDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\TileObstructionView.cpp
# End Source File
# Begin Source File

SOURCE=.\TileObstructionView.hpp
# End Source File
# Begin Source File

SOURCE=.\TilePalette.cpp
# End Source File
# Begin Source File

SOURCE=.\TilePalette.hpp
# End Source File
# Begin Source File

SOURCE=.\TilePropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TilePropertiesDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\TilesetEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\TilesetEditView.hpp
# End Source File
# Begin Source File

SOURCE=.\TilesetSelectionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TilesetSelectionDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\TilesetView.cpp
# End Source File
# Begin Source File

SOURCE=.\TilesetView.hpp
# End Source File
# Begin Source File

SOURCE=.\ToolPalette.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolPalette.hpp
# End Source File
# Begin Source File

SOURCE=.\VScrollWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\VScrollWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\Widget.cpp
# End Source File
# Begin Source File

SOURCE=.\Widget.hpp
# End Source File
# Begin Source File

SOURCE=.\WindowCommands.hpp
# End Source File
# Begin Source File

SOURCE=.\WindowStylePropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowStylePropertiesDialog.hpp
# End Source File
# Begin Source File

SOURCE=.\WindowStyleWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowStyleWindow.hpp
# End Source File
# Begin Source File

SOURCE=.\ZoneEditDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ZoneEditDialog.hpp
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# End Group
# Begin Group "Sphere Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\AnimationFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\common\common_palettes.cpp
# End Source File
# Begin Source File

SOURCE=..\common\configfile.cpp
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

SOURCE=..\common\endian.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Entities.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Filters.cpp
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

SOURCE=..\common\Image32.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Layer.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\common\LogDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\common\LogFile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\LogFile.hpp
# End Source File
# Begin Source File

SOURCE=..\common\LogNull.cpp
# End Source File
# Begin Source File

SOURCE=..\common\LogNull.hpp
# End Source File
# Begin Source File

SOURCE=..\common\LogWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\common\LogWindow.hpp
# End Source File
# Begin Source File

SOURCE=..\common\Map.cpp
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

SOURCE=..\common\packed_struct.h
# End Source File
# Begin Source File

SOURCE=..\common\Spriteset.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Tile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Tileset.cpp
# End Source File
# Begin Source File

SOURCE=..\common\types.h
# End Source File
# Begin Source File

SOURCE=..\common\WindowStyle.cpp
# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\engine\win32\win32_sphere_config.cpp
# End Source File
# End Group
# End Target
# End Project
