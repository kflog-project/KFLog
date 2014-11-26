REM***********************************************************************
REM
REM   RemoveKFLogRegistryKey
REM
REM   this commands script removes the standard KFLog Registry Key where
REM   user defined settings are stored.
REM
REM   This file is part of KFLog.
REM
REM***********************************************************************
REM
REM   Copyright (c):  2014 by psimon.kflog@gmail.com
REM
REM   This file is distributed under the terms of the General Public
REM   Licence. See the file COPYING for more information.
REM
REM**********************************************************************/
@cls
@echo This batch file deletes the KFLog User registry key
@echo.
@echo.	HKEY_CURRENT_USER\Sofware\KFLog
@echo.
@echo this key contains user specific setttings specified in Settings - KFLog Setup... 
@echo.
@set /P Answer=Should I delete HKEY_CURRENT_USER\Software\KFLog from the user-registry ? [y/n]: 
@if "%Answer%" == "y" (
	reg DELETE HKCU\Software\KFLog /f 
	) else (
	echo.
	echo registry key HKEY_CURRENT_USER\Software\KFLog was not deleted
	)
