#!/bin/bash
# Sonic 3 & Knuckles (static recomp) - RetroPie port
GAMEDIR="/home/pi/SonicOrigins"
cd "$GAMEDIR" || exit 1
exec ./launcher --launch 2
