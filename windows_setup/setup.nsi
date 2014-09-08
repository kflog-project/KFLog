; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "KFlog"

; The file to write
OutFile "KFlogSetup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\KFlog

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\KFlog" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel highest



LicenseText "Welcome to KFlog 4.7.3 BETA Setup"
LicenseData "License.rtf"
;--------------------------------

; Pages
PageEx license
   LicenseData License_beta.rtf
   LicenseForceSelection checkbox
PageExEnd
Page license 
Page components
Page directory

Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "KFlog program (required)"



  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File ${ExeSourcePath}\kflog.exe
  File ${MingwBinPath}\libgcc_s_dw2-1.dll
  File ${MingwBinPath}\libwinpthread-1.dll
  File ${MingwBinPath}\libstdc++-6.dll
  File ${QtBinPath}\QtCore4.dll
  File ${QtBinPath}\QtGui4.dll
  File ${QtBinPath}\QtNetwork4.dll
  File ${QtBinPath}\QtOpenGL4.dll
  File ${QtBinPath}\QtXml4.dll
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\KFlog "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KFlog" "DisplayName" "KFlog"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KFlog" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KFlog" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KFlog" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\KFlog"
  CreateShortCut "$SMPROGRAMS\KFlog\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\KFlog\KFlog.lnk" "$INSTDIR\Kflog.exe" "" "$INSTDIR\Kflog.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
    MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to remove the user settings directory $PROFILE\KFlog and all of its contents?$\r$\nThis will also delete all manually downloaded files like tiles or airspace files." IDNO NoDelete
    Delete "$PROFILE\KFlog\*.*"
    RMDir /r "$PROFILE\KFlog" ; skipped if no
NoDelete:
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KFlog"
  DeleteRegKey HKLM SOFTWARE\KFlog

  ; Remove files and uninstaller
    ;files
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\kflog.exe"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"
  Delete "$INSTDIR\QtOpenGL4.dll"
  Delete "$INSTDIR\QtXml4.dll"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\KFlog\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\KFlog"
  RMDir "$INSTDIR"

SectionEnd
