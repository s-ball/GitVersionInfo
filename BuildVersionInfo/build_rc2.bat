@ECHO OFF
REM This script expects 4 parameters:
REM  %1 : full target path of BuildVersionInfo executable
REM  %2 : file name of the executable (eg: BuildVersionInfo.exe)
REM  %3 : path to the version.ini file
REM  %4 : path to the versioninfo.rc2 file
REM It builds a complete versioninfo.rc2 file from version.ini
REM  if a previous version of the BuildVersionInfo executable exists.
REM Else if just build an empty (but valid) file.
IF EXIST %1 (
	%1 %2 /I:%3 %4
) ELSE (
	COPY /B NUL %4
)
