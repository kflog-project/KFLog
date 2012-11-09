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


LicenseText "Welcome to KFlog 4.6.2 Setup"
LicenseData "License.rtf"
;--------------------------------

; Pages
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
  File "Kflog4Windows.exe"
  File "mingwm10.dll"
  File "QtCore4.dll"
  File "QtGui4.dll"
  File "QtNetwork4.dll"
  File "QtOpenGL4.dll"
  File "QtXml4.dll"
  
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
  CreateShortCut "$SMPROGRAMS\KFlog\KFlog.lnk" "$INSTDIR\Kflog4Windows.exe" "" "$INSTDIR\Kflog4Windows.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KFlog"
  DeleteRegKey HKLM SOFTWARE\KFlog

  ; Remove files and uninstaller
    ;files
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\Kflog4Windows.exe"
  Delete "$INSTDIR\mingwm10.dll"
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
