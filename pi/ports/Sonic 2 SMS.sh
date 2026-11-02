#!/bin/bash
# Sonic 2 (Master System recomp) - RetroPie port
GAMEDIR="/home/pi/SonicOrigins"
cd "$GAMEDIR" || exit 1
exec ./launcher --launch 6
