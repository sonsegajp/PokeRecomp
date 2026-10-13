@echo off
rem Host a Hoenn world on this PC, with the rules read from your own Emerald.
rem Anyone can host one: it shows up in every player's ONLINE CLUB server
rem browser. Players' logins never reach this PC (the master hands each one
rem a one-time pass for this world only).
rem The world's settings (name, players, shiny odds, outbreaks, rules) are in
rem "Hoenn World Settings.ini": the first run makes it and opens it for you.
setlocal
cd /d "%~dp0"
title PokeRecomp - Hoenn world (Emerald)

set "PY="
where py >nul 2>nul && set "PY=py -3"
if not defined PY where python >nul 2>nul && set "PY=python"
if not defined PY (
    echo Hosting needs Python 3 ^(free^): https://www.python.org/downloads/
    echo When installing, tick "Add python.exe to PATH", then run this again.
    pause
    exit /b 1
)
if not exist "emerald.pkmn" (
    echo Import your Emerald ROM in the PokeRecomp launcher first:
    echo the world reads its rules ^(Pokemon, moves, wild areas^) from it.
    pause
    exit /b 1
)

if not exist "Hoenn World Settings.ini" (
    %PY% server\gameserver.py --region hoenn --port 27201 --name "%USERNAME%'s Hoenn" --write-config "Hoenn World Settings.ini"
    echo Your world's settings are in "Hoenn World Settings.ini".
    echo Change anything you like ^(name, players, shiny odds, outbreaks, rules^),
    echo save, and close Notepad to start the world.
    start "" /wait notepad "Hoenn World Settings.ini"
)

echo Reading the rules from your Emerald...
del "server\legality_data_em.json" 2>nul
start "" /wait PokeRecompGame.exe --game emerald --headless --dump-legality "server\legality_data_em.json"
if not exist "server\legality_data_em.json" (
    echo Couldn't read the rules from your Emerald.
    pause
    exit /b 1
)

echo.
echo Players reach this PC on the port in "Hoenn World Settings.ini": forward it on your
echo router and allow it through Windows Firewall if asked.
echo Close this window to take the world down.
echo.
%PY% server\gameserver.py --config "Hoenn World Settings.ini" --region hoenn
pause
