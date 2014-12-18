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

!verbose 4

LicenseText "Welcome to KFlog 4.10.0 Setup"
LicenseData WindowsReleaseNotes.rtf
;--------------------------------

; Pages
PageEx license
   LicenseData "License.rtf"
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
  File ${KflogProjDir}\LICENSE
  File ${KflogProjDir}\windows_setup\License.rtf
  File ${KflogProjDir}\windows_setup\WindowsReleaseNotes.rtf
  File ${KflogProjDir}\Changelog
  File ${MingwBinPath}\libgcc_s_dw2-1.dll
  File ${MingwBinPath}\..\mingw32\opt\bin\libwinpthread-1.dll
  File ${MingwBinPath}\libstdc++-6.dll
  File ${QtBinPath}\QtCore4.dll
  File ${QtBinPath}\QtGui4.dll
  File ${QtBinPath}\QtNetwork4.dll
  File ${QtBinPath}\QtOpenGL4.dll
  File ${QtBinPath}\QtXml4.dll

  SetOutPath $INSTDIR\translations
  File ${KflogProjDir}\kflog\kflog_de.qm
  File ${QtBinPath}\..\translations\qt_de.qm
  
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
  ; CreateDirectory "$SMPROGRAMS\KFlog\translations"
  CreateShortCut "$SMPROGRAMS\KFlog\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\KFlog\KFlog.lnk" "$INSTDIR\Kflog.exe" "" "$INSTDIR\Kflog.exe" 0
  CreateShortCut "$SMPROGRAMS\KFlog\KFlog License.lnk" "$INSTDIR\License.rtf" "" "$INSTDIR\License.rtf" 0
  CreateShortCut "$SMPROGRAMS\KFlog\KFlog Releasenotes.lnk" "$INSTDIR\WindowsReleaseNotes.rtf" "" "$INSTDIR\WindowsReleaseNotes.rtf" 0
  CreateShortCut "$SMPROGRAMS\KFlog\GPL 3.0.lnk" "notepad.exe" "$INSTDIR\LICENSE" "Gnu Public License 3.0 agreement" 0
  CreateShortCut "$SMPROGRAMS\KFlog\KFlog 4.10.0 Changelog.lnk" "notepad.exe" "$INSTDIR\Changelog" "KFLog 4.10.0 Changelog" 0
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
    MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to remove the user settings directory $PROFILE\KFlog and all of its contents?$\r$\nThis will also delete all manually downloaded files like tiles or airspace files." IDNO NoDelete
    Delete "$PROFILE\KFlog\*.*"
    RMDir /r "$PROFILE\KFlog" ; skipped if no
    DeleteRegKey HKCU SOFTWARE\KFlog
NoDelete:
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KFlog"
  DeleteRegKey HKLM SOFTWARE\KFlog

  ; Remove files and uninstaller
    ;files
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\kflog.exe"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\License.rtf"
  Delete "$INSTDIR\WindowsReleaseNotes.rtf"
  Delete "$INSTDIR\Changelog"
  Delete "$INSTDIR\translations\kflog_de.qm"
  Delete "$INSTDIR\translations\qt_de.qm"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\libwinpthread-1.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"
  Delete "$INSTDIR\QtOpenGL4.dll"
  Delete "$INSTDIR\QtXml4.dll"

  RMDir "$INSTDIR\translations"
  RMDir "$INSTDIR"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\KFlog\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\KFlog"
  RMDir "$INSTDIR"

SectionEnd
