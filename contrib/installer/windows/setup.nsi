Name ${PRODUCT_NAME}
BrandingText "${PRODUCT_NAME} ${PRODUCT_VERSION}"

!define SHORT_PRODUCT_NAME ${PRODUCT_NAME}
!define SHORT_PRODUCT_PATH ${PRODUCT_NAME}
!define PRODUCT_PUBLISHER "CaveProductions"
!define PRODUCT_WEB_SITE "http://www.caveproductions.org"
!define PRODUCT_DIR_REGKEY "Software\${PRODUCT_NAME}\${PRODUCT_NAME}"
!define GAME_EXPLORER

SetCompressor /FINAL /SOLID lzma
SetCompressorDictSize 64
RequestExecutionLevel admin

!include "MUI2.nsh"

!ifdef GAME_EXPLORER
!AddPluginDir "./plugins"
!endif

ShowInstDetails "nevershow"
ShowUninstDetails "nevershow"

; MUI Settings
!define MUI_ICON "..\..\icon.ico"
!define MUI_UNICON "..\..\icon.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "project.bmp"

!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKCU
!define MUI_STARTMENUPAGE_REGISTRY_KEY ${PRODUCT_DIR_REGKEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER ${PRODUCT_NAME}

; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PRODUCT_NAME}.exe"
!define MUI_FINISHPAGE_RUN_NOTCHECKED

!define MUI_LICENSEPAGE_RADIOBUTTONS

!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

Var StartMenuGroup

; Store the language
!define MUI_LANGDLL_REGISTRY_ROOT HKCU
!define MUI_LANGDLL_REGISTRY_KEY ${PRODUCT_DIR_REGKEY}
!define MUI_LANGDLL_REGISTRY_VALUENAME "InstallerLanguage"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Installer languages
!insertmacro MUI_LANGUAGE "English"

!insertmacro MUI_RESERVEFILE_LANGDLL
; MUI end ------

OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKCU "${PRODUCT_DIR_REGKEY}" "InstallPath"

Section "${PRODUCT_NAME}" SecMain
	SetOverwrite on

	SetOutPath "$INSTDIR\base"
		File /r "..\..\..\base\"

	SetOutPath "$INSTDIR"
		File "..\..\..\${PRODUCT_NAME}.exe"

	WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" InstallPath "$INSTDIR"

!ifdef GAME_EXPLORER
	#Register with game explorer
	Games::registerGame "$INSTDIR\${PRODUCT_NAME}.exe"
	pop $0
	# This is for Vista only, see the gameexplorer.xml for higher versions
	${If} $0 != "0"
	${AndIf} $0 != ""
	${AndIf} $0 != "$INSTDIR\${PRODUCT_NAME}.exe"
		CreateDirectory "$0\PlayTasks\0"
		CreateShortcut "$0\PlayTasks\0\Play.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
		CreateDirectory "$0\SupportTasks\0"
		CreateShortcut "$0\SupportTasks\0\Home Page.lnk" "${PRODUCT_WEB_SITE}"
	${EndIf}
!endif

SectionEnd

Section -post SecMainPost
	SetOutPath $INSTDIR

	WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
	WriteUninstaller $INSTDIR\uninstall.exe

	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	SetShellVarContext all ; Create shortcuts in the all-users folder
	CreateDirectory "$SMPROGRAMS\$StartMenuGroup"
	CreateShortCut "$SMPROGRAMS\$StartMenuGroup\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe" "" "$INSTDIR\${PRODUCT_NAME}.exe" 0
	CreateShortCut "$SMPROGRAMS\$StartMenuGroup\${PRODUCT_NAME} Editor.lnk" "$INSTDIR\${PRODUCT_NAME}.exe" "-ui_push editor" "$INSTDIR\${PRODUCT_NAME}.exe" 0
	CreateShortCut "$SMPROGRAMS\$StartMenuGroup\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
	CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Uninstall ${PRODUCT_NAME}.lnk" $INSTDIR\uninstall.exe
	!insertmacro MUI_STARTMENU_WRITE_END

	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" DisplayName "${PRODUCT_NAME}"
	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" DisplayVersion "${PRODUCT_VERSION}"
	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" Publisher "${PRODUCT_PUBLISHER}"
	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" URLInfoAbout "${PRODUCT_WEB_SITE}"
	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" DisplayIcon $INSTDIR\uninstall.exe
	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" UninstallString $INSTDIR\uninstall.exe
	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" InstallLocation $INSTDIR
	WriteRegDWORD HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" NoModify 1
	WriteRegDWORD HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" NoRepair 1
SectionEnd

Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Section -un.Main SecUninstall
	!ifdef GAME_EXPLORER
		Games::unregisterGame "$INSTDIR\${PRODUCT_NAME}.exe"
	!endif
SectionEnd

Section -un.post SecUninstallPost
	SetShellVarContext all
	RmDir /r /REBOOTOK $SMPROGRAMS\$StartMenuGroup
	RmDir /r /REBOOTOK $INSTDIR

	DeleteRegKey HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
	DeleteRegValue HKCU "${PRODUCT_DIR_REGKEY}" StartMenuGroup
	DeleteRegValue HKCU "${PRODUCT_DIR_REGKEY}" InstallPath
	DeleteRegValue HKCU "${PRODUCT_DIR_REGKEY}" InstallerLanguage
	DeleteRegKey /IfEmpty HKCU "${PRODUCT_DIR_REGKEY}"
SectionEnd

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
	ReadRegStr $INSTDIR HKCU "${PRODUCT_DIR_REGKEY}" InstallPath
	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup
FunctionEnd
