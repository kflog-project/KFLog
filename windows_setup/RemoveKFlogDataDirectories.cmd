REM***********************************************************************
REM
REM   RemoveKFLogDataDirectories
REM
REM   this commands script removes the standard KFLog user data directories
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
@echo This batch file deletes the KFLog standard directory structure of user data:
@echo.
@echo.	%userprofile%\KLFOG
@echo.
@echo If you have specified additional or different other directories in 
@echo Settings - KFLog Setup... you need to delete these files manually!
@echo.
@set /P Answer=Should I delete %userprofile%\KFLog [y/n]?: 
@if "%Answer%" == "y" (
	rd /q /s %userprofile%\KFLOG
	) else (
	echo.
	echo directory %userprofile%\KFLog was not deleted
	)