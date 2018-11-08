; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "EasyVoiceCall"

; The file to write
OutFile "EasyVoiceCall.Client.Windows.Installer.exe"

; The default installation directory
InstallDir $PROGRAMFILES\EasyVoiceCall

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "EasyVoiceCall (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File /r artifacts\*.*
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\EasyVoiceCall "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EasyVoiceCall" "DisplayName" "EasyVoiceCall"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EasyVoiceCall" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EasyVoiceCall" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EasyVoiceCall" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\EasyVoiceCall"
  CreateShortcut "$SMPROGRAMS\EasyVoiceCall\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\EasyVoiceCall\EasyVoiceCall.lnk" "$INSTDIR\EasyVoiceCall.exe" "" "$INSTDIR\EasyVoiceCall.exe" 0
  CreateShortCut "$DESKTOP\EasyVoiceCall.lnk" "$INSTDIR\EasyVoiceCall.exe" ""
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EasyVoiceCall"
  DeleteRegKey HKLM SOFTWARE\EasyVoiceCall

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\EasyVoiceCall\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\EasyVoiceCall"
  RMDir /r "$INSTDIR"

SectionEnd
