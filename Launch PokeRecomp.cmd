@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch PokeRecomp.ps1" %*
if errorlevel 1 pause
