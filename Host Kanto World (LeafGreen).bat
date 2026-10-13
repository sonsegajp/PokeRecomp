@echo off
rem Host a Kanto world on this PC, with the rules read from your own LeafGreen.
rem Anyone can host one: it shows up in every player's ONLINE CLUB server
rem browser. Players' logins never reach this PC (the master hands each one
rem a one-time pass for this world only).
setlocal
cd /d "%~dp0"
title PokeRecomp - Kanto world (LeafGreen)

set "PY="
where py >nul 2>nul && set "PY=py -3"
if not defined PY where python >nul 2>nul && set "PY=python"
if not defined PY (
    echo Hosting needs Python 3 ^(free^): https://www.python.org/downloads/
    echo When installing, tick "Add python.exe to PATH", then run this again.
    pause
    exit /b 1
)
if not exist "leafgreen.pkmn" (
    echo Import your LeafGreen ROM in the PokeRecomp launcher first:
    echo the world reads its rules ^(Pokemon, moves, wild areas^) from it.
    pause
    exit /b 1
)

set "NAME=%USERNAME%'s Kanto"
set /p "NAME=World name [%NAME%]: "
set "PORT=27200"
set /p "PORT=Port [27200]: "

echo Reading the rules from your LeafGreen...
del "server\legality_data_lg.json" 2>nul
start "" /wait PokeRecompGame.exe --game leafgreen --headless --dump-legality "server\legality_data_lg.json"
if not exist "server\legality_data_lg.json" (
    echo Couldn't read the rules from your LeafGreen.
    pause
    exit /b 1
)

echo.
echo Players reach this PC on TCP port %PORT%: forward it on your router
echo and allow it through Windows Firewall if asked.
echo Close this window to take the world down.
echo.
%PY% server\gameserver.py --region kanto --name "%NAME%" --port %PORT%
pause
