;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "uGet"
  OutFile "uget-no-gtk.exe"

  ;Default installation folder
; InstallDir "$LOCALAPPDATA\uGet"
  InstallDir "$PROGRAMFILES\uGet"

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\uGet" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Variables

  ; MUI_PAGE_STARTMENU
  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_HEADERIMAGE
; !define MUI_HEADERIMAGE_BITMAP ".\pixmaps\uget-icon-32x32.bmp" ; optional

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE ".\License-install.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\uGet" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  ; Function
  Page custom DetectAndSetupGTK

  !insertmacro MUI_PAGE_INSTFILES

  ;Finish Page config
  !define MUI_FINISHPAGE_RUN			"$INSTDIR\uget.exe"
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "uGet program" uGetProgram

  ; All file and folder placed in .\archive
  SetOutPath "$INSTDIR"
  File /r .\archive\*.*

  ; main program
  File ..\CodeBlocks\Release\uget.exe
  ; icons
  SetOutPath "$INSTDIR\icons\hicolor\"
  File ..\..\pixmaps\icons\hicolor\index.theme
  SetOutPath "$INSTDIR\icons\hicolor\16x16\apps"
  File ..\..\pixmaps\icons\hicolor\16x16\apps\uget-icon.png
  File ..\..\pixmaps\icons\hicolor\16x16\apps\uget-error.png
  File ..\..\pixmaps\icons\hicolor\16x16\apps\uget-downloading.png
  SetOutPath "$INSTDIR\icons\hicolor\24x24\apps"
  File ..\..\pixmaps\icons\hicolor\24x24\apps\uget-icon.png
  File ..\..\pixmaps\icons\hicolor\24x24\apps\uget-error.png
  File ..\..\pixmaps\icons\hicolor\24x24\apps\uget-downloading.png
  SetOutPath "$INSTDIR\icons\hicolor\32x32\apps"
  File ..\..\pixmaps\icons\hicolor\32x32\apps\uget-icon.png
  File ..\..\pixmaps\icons\hicolor\32x32\apps\uget-error.png
  File ..\..\pixmaps\icons\hicolor\32x32\apps\uget-downloading.png
  SetOutPath "$INSTDIR\icons\hicolor\48x48\apps"
  File ..\..\pixmaps\icons\hicolor\48x48\apps\uget-icon.png
  File ..\..\pixmaps\icons\hicolor\48x48\apps\uget-error.png
  File ..\..\pixmaps\icons\hicolor\48x48\apps\uget-downloading.png
  SetOutPath "$INSTDIR\icons\hicolor\64x64\apps"
  File ..\..\pixmaps\icons\hicolor\64x64\apps\uget-icon.png
  SetOutPath "$INSTDIR\icons\hicolor\128x128\apps"
  File ..\..\pixmaps\icons\hicolor\128x128\apps\uget-icon.png
  SetOutPath "$INSTDIR\icons\hicolor\scalable\apps\"
  File ..\..\pixmaps\icons\hicolor\scalable\apps\uget-icon.svg
  ; pixmaps
; SetOutPath "$INSTDIR\pixmaps\uget"
; File ..\..\pixmaps\logo.png
  ; sounds
  SetOutPath "$INSTDIR\sounds\uget"
  File ..\..\sounds\notification.wav

  ;Store installation folder
  WriteRegStr HKLM "Software\uGet" "" $INSTDIR

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\uGet" "DisplayName" "uGet"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\uGet" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\uGet" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\uGet" "NoRepair" 1

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; MUI_PAGE_STARTMENU
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\uGet.lnk" "$INSTDIR\uget.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_uGetProgram ${LANG_ENGLISH} "Install uGet main program."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${uGetProgram} $(DESC_uGetProgram)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; Remove the uninstall keys for Windows
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\uGet"

  ; Remove registry keys (installation folder and Path)
  DeleteRegKey HKLM SOFTWARE\uGet
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\uget.exe"

  ;ADD YOUR OWN FILES HERE...
  Delete $INSTDIR\uget.exe
  ; libcurl & OpenSSL
  Delete $INSTDIR\libcurl.dll
  Delete $INSTDIR\libeay32.dll
  Delete $INSTDIR\libidn-11.dll
  Delete $INSTDIR\libssh2.dll
  Delete $INSTDIR\libssl32.dll
  Delete $INSTDIR\zlib1.dll
  ; License
  Delete $INSTDIR\COPYING
  Delete $INSTDIR\COPYING.libcurl
  Delete $INSTDIR\LICENSE.OpenSSL
  ; delete folder
  RMDir /r "$INSTDIR\pixmaps"
  RMDir /r "$INSTDIR\icons"
  RMDir /r "$INSTDIR\locale"
  RMDir /r "$INSTDIR\sounds"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  ; MUI_PAGE_STARTMENU
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\uGet.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"


  DeleteRegKey /ifempty HKLM "Software\uGet"

SectionEnd


; -----------------------------------------------------------------------------
; Functions

Function DetectAndSetupGTK

  ReadRegStr $0 HKLM "Software\GTK\3.0" Version

  ; StrCmp str1 str2 jump_if_equal [jump_if_not_equal]
  StrCmp $0 "" GtkNotInst GtkInst

  GtkNotInst:
  MessageBox MB_OK|MB_ICONSTOP "GTK+ Runtime Environment version 3.4.x or newer is required to use uGet.$\n$\nYou can download it from http://gtk-win.sourceforge.net/"
  Quit

  GtkInst:
  ; StrCpy var1 var2 len position
  ; Check major version
  StrCpy $1 $0 1 0
  ${If} $1 < 3
    Goto GtkNotInst
  ${EndIf}
  ; Check minor version
  StrCpy $1 $0 2 2
  ${If} $1 >= 4
    Goto DoReg
  ${Else}
    Goto GtkNotInst
  ${EndIf}

  DoReg:
  ReadRegStr $2 HKLM "Software\GTK\3.0" DllPath
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\uget.exe" "" "$INSTDIR\uget.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\uget.exe" "Path" "$INSTDIR;$2"

FunctionEnd
